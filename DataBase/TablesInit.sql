CREATE DATABASE IF NOT EXISTS DronePath3D;

USE DronePath3D;

CREATE TABLE IF NOT EXISTS Scenes (
    SceneID INT AUTO_INCREMENT PRIMARY KEY COMMENT '场景唯一标识符',
    Name VARCHAR(255) NOT NULL COMMENT '场景名称',
    Description VARCHAR(500) COMMENT '场景描述',
    PointCloudDataPath VARCHAR(1000) COMMENT '点云数据文件路径'
) COMMENT='场景信息表';
