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
        // �������ݿ�����
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // ѡ�����ݿ�
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // ��ȡ�ļ�����Ϊ Name
        std::string sceneName = TCHAR_TO_UTF8(*FPaths::GetBaseFilename(FilePath));

        // 1. ����Ƿ��Ѿ�������ͬ�� Name
        std::string checkQuery = "SELECT COUNT(*) FROM Scenes WHERE Name = ?";
        std::unique_ptr<sql::PreparedStatement> checkStmt(con->prepareStatement(checkQuery));
        checkStmt->setString(1, sceneName);

        // ִ�в�ѯ
        std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());
        res->next();

        // �������� 1�����ʾ�Ѿ�������ͬ�� Name������ false
        if (res->getInt(1) > 0) {
            UE_LOG(LogTemp, Warning, TEXT("Scene with name '%s' already exists."), *FString(sceneName.c_str()));
            return false;
        }

        // 2. ���û���ظ��� Name��ִ�в������
        std::string query = "INSERT INTO Scenes (Name, Description, PointCloudDataPath) VALUES (?, ?, ?)";
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

        // ���ò���
        pstmt->setString(1, sceneName); // ���� Name
        pstmt->setString(2, "description"); // ���� Description
        pstmt->setString(3, TCHAR_TO_UTF8(*(FPaths::ProjectDir() + TEXT("Data/PointClouds/") + FilePath))); // ���� PointCloudDataPath

        // ִ�в���
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
        // �������ݿ�����
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // ѡ�����ݿ�
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // ����ɾ�����
        std::string query = "DELETE FROM Scenes WHERE SceneID = ?";

        // ʹ��Ԥ�������
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

        // ���ò���
        pstmt->setInt(1, SceneID);

        // ִ��ɾ������
        int rowsAffected = pstmt->executeUpdate();

        // ����Ƿ��м�¼��ɾ��
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
        // �������ݿ�����
        std::unique_ptr<sql::Connection> con(driver->connect(HostName, UserName, Password));

        // ѡ�����ݿ�
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // ִ�в�ѯ���
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM Scenes"));

        // ���������
        while (res->next()) {
            // ����һ���µ� SceneInfo ����
            USceneInfo* sceneInfo = NewObject<USceneInfo>();

            // ��ȡ��ѯ�������� USceneInfo ����
            sceneInfo->SceneID = res->getInt("SceneID");
            sceneInfo->SceneName = FString(res->getString("Name").c_str());
            sceneInfo->PointCloudDataPath = FString(res->getString("PointCloudDataPath").c_str());

            // �� SceneInfo ������ӵ� TArray
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
    

    // �������ļ��ж�ȡ
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
    stamt->execute("CREATE TABLE IF NOT EXISTS Scenes ("
        "SceneID INT AUTO_INCREMENT PRIMARY KEY,"
        "Name VARCHAR(255) NOT NULL,"
        "Description VARCHAR(500),"
        "PointCloudDataPath VARCHAR(1000)"
        "); ");
    stamt->execute("CREATE TABLE IF NOT EXISTS Drones ("
        "DroneID INT AUTO_INCREMENT PRIMARY KEY,"
        "SceneID INT,"
        "StartX FLOAT,"
        "StartY FLOAT,"
        "StartZ FLOAT,"
        "Name VARCHAR(255),"
        "MaxSpeed FLOAT,"
        "MaxHeight FLOAT,"
        "Endurance FLOAT,"
        "Weight FLOAT,"
        "MaxThrust FLOAT,"
        "Diameter FLOAT,"
        "Height FLOAT,"
        "FOREIGN KEY(SceneID) REFERENCES scenes(SceneID)"
        "); ");
}

UDatabaseHelper::UDatabaseHelper() {

}

