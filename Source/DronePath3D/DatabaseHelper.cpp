// Fill out your copyright notice in the Description page of Project Settings.


#include "DatabaseHelper.h"

#define HOSTNAME "tcp://localhost:3306"
#define USERNAME "root"
#define PASSWORD "xzf"

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
        std::unique_ptr<sql::Connection> con(driver->connect(HOSTNAME, USERNAME, PASSWORD));
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
        std::unique_ptr<sql::Connection> con(driver->connect(HOSTNAME, USERNAME, PASSWORD));

        // ѡ�����ݿ�
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("USE DronePath3D");

        // �����������
        std::string query = "INSERT INTO Scenes (Name, Description, PointCloudDataPath) VALUES (?, ?, ?)";

        // ʹ��Ԥ�������
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

        // ���ò���
        pstmt->setString(1, TCHAR_TO_UTF8(*FPaths::GetBaseFilename(FilePath)));                // ���� Name
        pstmt->setString(2, "description");         // ���� Description
        pstmt->setString(3, TCHAR_TO_UTF8(*(FPaths::ProjectDir() + TEXT("Data/PointClouds/") + FilePath)));  // ���� PointCloudDataPath

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
        std::unique_ptr<sql::Connection> con(driver->connect(HOSTNAME, USERNAME, PASSWORD));

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
        std::unique_ptr<sql::Connection> con(driver->connect(HOSTNAME, USERNAME, PASSWORD));

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
	driver = sql::mysql::get_mysql_driver_instance();
    std::unique_ptr<sql::Connection> con(driver->connect(HOSTNAME, USERNAME, PASSWORD));
    std::unique_ptr<sql::Statement> stamt(con->createStatement());
    stamt->execute("CREATE DATABASE IF NOT EXISTS DronePath3D");
    stamt->execute("USE DronePath3D");
}

UDatabaseHelper::UDatabaseHelper() {

}

#undef HOSTNAME
#undef USERNAME
#undef PASSWORD
