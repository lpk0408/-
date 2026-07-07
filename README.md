# 第九届嵌入式芯片与系统设计竞赛/软通赛道/宠物关怀
## 云宠灵犀—基于rk2206+OHOS多模态AI宠物智护系统
* **主控芯片**：RK2206 (运行 OpenHarmony / LiteOS)
* **从机节点 1**：STM32F407 移植Huawei LiteOS操作系统 (喂粮从机)
* **从机节点 2**：STM32F407 移植Huawei LiteOS操作系统 (穿戴从机)
* **[关于LiteOS操作系统移植教程链接](https://www.cnblogs.com/tianbeibei/articles/18740766)** *
## 项目介绍
### 首先主控是txsmartrk2206开发板，基于rk2206芯片和openharmony实时操作系统，额外搭载LoRa模块，以及三个继电器来控制风扇(5V/0.3A)、加湿器(QDB1)、窗帘（舵机SG90）
