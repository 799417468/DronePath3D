// Fill out your copyright notice in the Description page of Project Settings.


#include "DatabaseHelper.h"

UDatabaseHelper* UDatabaseHelper::GetSingleton()
{
	if (!Singleton) {
		Singleton = NewObject<UDatabaseHelper>();
		Singleton->AddToRoot();
		Singleton->Initialize();
	}
	return Singleton;
}

bool UDatabaseHelper::execute(FString sql)
{
    try {
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));
        std::unique_ptr<sql::Statement> stamt(con->createStatement());
        stamt->execute("USE DronePath3D");
        stamt->execute(TCHAR_TO_UTF8(*sql));
        return true;
    }
    catch (const sql::SQLException& e) {
        UE_LOG(LogTemp, Error, TEXT("SQL Exception: %s"), *FString(e.what()));
    }
    catch (const std::exception& e) {
        UE_LOG(LogTemp, Error, TEXT("Standard Exception: %s"), *FString(e.what()));
    }
    catch (...) {
        UE_LOG(LogTemp, Error, TEXT("SQL execute: Unknown Exception occurred."));
    }
    return false;
}

bool UDatabaseHelper::AddNewScene(FString FilePath)
{
    try {
        // 创建数据库连接
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // 选择数据库
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // 获取文件名作为 Name
        std::string sceneName = TCHAR_TO_UTF8(*FPaths::GetBaseFilename(FilePath));

        // 1. 检查是否已经存在相同的 Name
        std::string checkQuery = "SELECT COUNT(*) FROM Scenes WHERE Name = ?";
        std::unique_ptr<sql::PreparedStatement> checkStmt(con->prepareStatement(checkQuery));
        checkStmt->setString(1, sceneName);

        // 执行查询
        std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());
        res->next();

        // 如果结果是 1，则表示已经存在相同的 Name，返回 false
        if (res->getInt(1) > 0) {
            UE_LOG(LogTemp, Warning, TEXT("Scene with name '%s' already exists."), *FString(sceneName.c_str()));
            return false;
        }

        // 2. 如果没有重复的 Name，执行插入操作
        std::string query = "INSERT INTO Scenes (Name, Description, PointCloudDataPath) VALUES (?, ?, ?)";
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

        // 设置参数
        pstmt->setString(1, sceneName); // 设置 Name
        pstmt->setString(2, "description"); // 设置 Description
        pstmt->setString(3, TCHAR_TO_UTF8(*(FPaths::ProjectDir() + TEXT("Data/PointClouds/") + FilePath))); // 设置 PointCloudDataPath

        // 执行插入
        pstmt->executeUpdate();

        return true;
    }
    catch (const sql::SQLException& e) {
        UE_LOG(LogTemp, Error, TEXT("SQL Exception: %s"), *FString(e.what()));
    }
    catch (const std::exception& e) {
        UE_LOG(LogTemp, Error, TEXT("Standard Exception: %s"), *FString(e.what()));
    }
    catch (...) {
        UE_LOG(LogTemp, Error, TEXT("SQL execute: Unknown Exception occurred."));
    }
    return false;
}

bool UDatabaseHelper::DeleteScene(int32 SceneID)
{
    try {
        // 创建数据库连接
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // 选择数据库
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // 创建删除语句
        std::string query = "DELETE FROM Scenes WHERE SceneID = ?";

        // 使用预处理语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

        // 设置参数
        pstmt->setInt(1, SceneID);

        // 执行删除操作
        int rowsAffected = pstmt->executeUpdate();

        // 检查是否有记录被删除
        if (rowsAffected > 0) {
            UE_LOG(LogTemp, Log, TEXT("Scene with ID %d deleted successfully."), SceneID);
            return true;
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("No scene found with ID %d."), SceneID);
        }
    }
    catch (const sql::SQLException& e) {
        UE_LOG(LogTemp, Error, TEXT("SQL Exception: %s"), *FString(e.what()));
    }
    catch (const std::exception& e) {
        UE_LOG(LogTemp, Error, TEXT("Standard Exception: %s"), *FString(e.what()));
    }
    catch (...) {
        UE_LOG(LogTemp, Error, TEXT("SQL execute: Unknown Exception occurred."));
    }
    return false;
}

TArray<USceneInfo*> UDatabaseHelper::GetAllScenes()
{
    TArray<USceneInfo*> scenes;
    try {
        // 创建数据库连接
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // 选择数据库
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // 执行查询语句
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM Scenes"));

        // 遍历结果集
        while (res->next()) {
            // 创建一个新的 SceneInfo 对象
            USceneInfo* sceneInfo = NewObject<USceneInfo>();

            // 获取查询结果并填充 USceneInfo 对象
            sceneInfo->SceneID = res->getInt("SceneID");
            sceneInfo->SceneName = FString(res->getString("Name").c_str());
            sceneInfo->PointCloudDataPath = FString(res->getString("PointCloudDataPath").c_str());

            // 将 SceneInfo 对象添加到 TArray
            scenes.Add(sceneInfo);
        }

    }
    catch (const sql::SQLException& e) {
        UE_LOG(LogTemp, Error, TEXT("SQL Exception: %s"), *FString(e.what()));
    }
    catch (const std::exception& e) {
        UE_LOG(LogTemp, Error, TEXT("Standard Exception: %s"), *FString(e.what()));
    }
    catch (...) {
        UE_LOG(LogTemp, Error, TEXT("SQL execute: Unknown Exception occurred."));
    }

    return scenes;
}

void UDatabaseHelper::Initialize()
{
    FString Host, User, Pass;
    

    // 从配置文件中读取
    if (GConfig)
    {
        GConfig->GetString(TEXT("DatabaseSettings"), TEXT("Host"), Host, GGameIni);
        GConfig->GetString(TEXT("DatabaseSettings"), TEXT("User"), User, GGameIni);
        GConfig->GetString(TEXT("DatabaseSettings"), TEXT("Password"), Pass, GGameIni);
        this->HostName = TCHAR_TO_UTF8(*Host);
        this->UserName = TCHAR_TO_UTF8(*User);
        this->Password = TCHAR_TO_UTF8(*Pass);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GConfig is not available!"));
    }
	driver = sql::mysql::get_mysql_driver_instance();
    std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));
    std::unique_ptr<sql::Statement> stamt(con->createStatement());
    stamt->execute("CREATE DATABASE IF NOT EXISTS DronePath3D");
    stamt->execute("USE DronePath3D");
}

UDatabaseHelper::UDatabaseHelper() {

}

#undef HOSTNAME
#undef USERNAME
#undef PASSWORD
