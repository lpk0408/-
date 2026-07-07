# 🐾 云宠灵犀 — 基于 RK2206 + OpenHarmony 多模态 AI 宠物智护系统

> **CloudPet Lingxi** — A Multi-Modal AI Pet Smart Care System based on RK2206 + OpenHarmony OS

[![Platform](https://img.shields.io/badge/Platform-OpenHarmony%20%7C%20LiteOS-blue)](https://www.openharmony.cn/)
[![Language](https://img.shields.io/badge/Language-ArkTS%20%7C%20Java%20%7C%20C%2FC%2B%2B-orange)]()
[![AI](https://img.shields.io/badge/AI-YOLO%20%7C%20DeepSeek%20%7C%20RAG-brightgreen)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey)]()

---

## 📖 目录

- [项目简介](#项目简介)
- [系统架构](#系统架构)
- [核心特性](#核心特性)
- [硬件设计](#硬件设计)
- [软件设计](#软件设计)
- [AI 能力](#ai-能力)
- [技术指标](#技术指标)
- [应用场景](#应用场景)
- [性能测试](#性能测试)
- [快速开始](#快速开始)
- [项目结构](#项目结构)
- [发展展望](#发展展望)
- [参考文献](#参考文献)

---

## 项目简介

**云宠灵犀**是一款基于 **RK2206（OpenHarmony OS）**与多从机 **STM32F407（Huawei LiteOS）**协同工作的多模态 AI 宠物智能看护系统。系统采用 **"网关 + 从机 + 穿戴"** 分布式异构架构，融合物联网通信、边缘计算、深度学习视觉识别与大语言模型技术，打造从环境感知、身份识别、数据分析到智能决策与自动执行的全链路智慧宠物看护解决方案。

### 解决的核心痛点

| 痛点 | 解决方案 |
|------|----------|
| 🏠 外出时无法了解宠物状态 | 多传感器环境实时监测 + 远程视频/数据看板 |
| 🍖 无法科学喂食喂水 | 自动粮重/水位监测 + 阈值联动自动补粮补水 |
| 🐕 多宠家庭身份混淆 | YOLO + MobileNet + ArcFace 狗脸识别，"一宠一档" |
| 🏃 户外活动无法追踪 | GPS + NB-IoT 定位 + MPU6050 计步 + 电池监测 |
| 📚 缺乏专业喂养指导 | DeepSeek 大模型 + RAG 知识库 + Function Calling |

---

## 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                          📱 移动端 (ArkTS)                        │
│         首页看板 │ 设备控制 │ AI 对话 │ 知识库 │ 宠物档案          │
└──────────────────────────┬──────────────────────────────────────┘
                           │ HTTPS / MQTT
┌──────────────────────────▼──────────────────────────────────────┐
│                       ☁️ 云平台服务层                              │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────────┐    │
│  │ Spring Boot │  │  EMQX Broker │  │  AI 推理服务          │    │
│  │ REST API    │  │  MQTT 接入   │  │  YOLO + DogFace + RAG │    │
│  └─────────────┘  └──────────────┘  └──────────────────────┘    │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────────┐    │
│  │   Redis     │  │    MinIO     │  │  DeepSeek API        │    │
│  │ 缓存/知识库  │  │  对象存储    │  │  大语言模型           │    │
│  └─────────────┘  └──────────────┘  └──────────────────────┘    │
└──────────────────────────┬──────────────────────────────────────┘
                           │
          ┌────────────────┼────────────────────┐
          │ WiFi/MQTT      │ LoRa               │ NB-IoT
          ▼                ▼                    ▼
┌─────────────────┐ ┌──────────────┐ ┌──────────────────┐
│ 🖥️ RK2206 网关   │ │ 🍖 喂粮从机   │ │ ⌚ 穿戴从机       │
│ OpenHarmony OS  │ │ STM32F407    │ │ STM32F407        │
│                 │ │ LiteOS       │ │ LiteOS           │
│ • 温湿度 DHT22  │ │ • 水位传感器  │ │ • GPS ATGM336H   │
│ • 烟雾 MQ-2     │ │ • 称重 HX711  │ │ • MPU6050 计步   │
│ • 光照 BH1750   │ │ • SG90 舵机   │ │ • INA226 电池    │
│ • 继电器控制×4  │ │ • LoRa 通信   │ │ • NB-IoT QS-100  │
└─────────────────┘ └──────────────┘ └──────────────────┘
```

### 通信链路

| 链路 | 方式 | 用途 |
|------|------|------|
| 喂粮从机 ↔ 网关 | **LoRa** (433MHz, DX LCC68) | 水位/粮重数据上报、喂粮/喂水指令下发 |
| 网关 ↔ 云端 | **WiFi + MQTT** | 环境数据上报、控制指令接收 |
| 穿戴从机 ↔ 云端 | **NB-IoT** (QS-100) | GPS 位置、步数、电池数据独立上云 |
| 移动端 ↔ 云端 | **HTTPS + MQTT** | 数据看板、设备控制、AI 对话 |

---

## 核心特性

### 🧠 四大技术创新

#### 创新一：YOLO + MobileNet + ArcFace 狗脸识别多宠身份认证

首次将狗脸识别技术系统化应用于宠物智能看护领域：

1. **YOLO 目标检测** → 识别宠物状态（normal / sleeping / vomit / hotwet）并定位狗脸候选区域
2. **MobileNet 骨干 + ArcFace 损失** → 训练狗脸特征提取模型，输出 128 维归一化 Embedding
3. **ONNX Runtime 云端推理** → 余弦相似度 1:N 匹配，阈值 0.35
4. **Redis 宠物档案特征库** → 支持新增、批量识别、多宠状态标注

```text
图片 → YOLO 检测 (640×640) → 狗脸裁剪 (224×224) → MobileNet+ArcFace → 128维向量 → 余弦匹配 → 身份确认
```

#### 创新二：混合 RAG + Function Calling 的 AI 交流引擎

```
用户提问
    │
    ├──→ 倒排索引精确检索 (权重 0.45)
    ├──→ DashScope text-embedding-v4 向量语义检索 (权重 0.55)
    │
    ▼
加权融合排序 → Top-K 知识片段 → 注入 Prompt
    │
    ├──→ DeepSeek API 流式生成
    └──→ Function Calling → 环境状态/宠物档案/步数/睡眠/设备控制
```

- 支持上传 **PDF / DOC / DOCX** 专业知识文档
- 按页/段落切分（≤800字符），存入 Redis 知识库
- 精确匹配 + 语义模糊双路检索，有效降低大模型幻觉
- Function Calling 调用实时业务函数，实现"问答 + 实时状态 + 控制建议"闭环

#### 创新三：分布式异构多 OS 协同架构

| 层级 | 硬件 | 操作系统 | 职责 |
|------|------|----------|------|
| 网关层 | RK2206 (Cortex-A7) | **OpenHarmony** | 传感器采集、LoRa 解析、MQTT 上报、继电器控制 |
| 从机层 | STM32F407 (Cortex-M4) | **Huawei LiteOS** | 传感器采集、执行器控制、实时任务调度 |
| 穿戴层 | STM32F407 (Cortex-M4) | **Huawei LiteOS** | GPS 解析、计步算法、电池监测、NB-IoT 上报 |

三种无线通信方式（**LoRa + WiFi + NB-IoT**）实现网关-从机-云端三级互联。

#### 创新四：多模态感知融合 + 智能阈值联动

```
环境传感器 ─┐
生物传感器 ─┼─→ AI 模型分析 ─→ 阈值比对 ─→ 联动规则匹配 ─→ 自动控制
视觉传感器 ─┘                                              ├─ 风扇
GPS 定位   ──────────────────────────────────────          ├─ 遮光帘
                                                           ├─ 加湿器
                                                           └─ 喂粮/喂水
```

---

## 硬件设计

### 元器件清单

#### 🖥️ 网关主板 (RK2206)

| 组件 | 型号/参数 |
|------|-----------|
| 核心板 | RK2206 (ARM Cortex-A7, 1.2GHz) |
| LoRa 模组 | DX LCC68, 433MHz |
| 温湿度传感器 | DHT22 (±0.5°C, ±2%RH) |
| 烟雾传感器 | MQ-2 |
| 光照传感器 | BH1750 |
| 显示屏 | LCD |
| 继电器 | ×4 (风扇 / 遮光帘 / 加湿器 / 备用) |

#### 🍖 喂粮从机 (STM32F407)

| 组件 | 型号/参数 |
|------|-----------|
| 主控芯片 | STM32F407VET6 (Cortex-M4, 168MHz) |
| LoRa 模组 | DX LCC68, 433MHz |
| 水位传感器 | Water Sensor (精度 ±2mm) |
| 称重传感器 | HX711 ADC (精度 ±1g, 量程 0-5000g, R²=0.998) |
| 储粮开关 | SG90 舵机 (PWM 控制) |
| 供水 | 5V 舵机 + 继电器 |
| 电源 | AMS1117-3.3V/5V 稳压 |

#### ⌚ 穿戴从机 (STM32F407)

| 组件 | 型号/参数 |
|------|-----------|
| 主控芯片 | STM32F407VET6 |
| GPS 模组 | ATGM336H (精度 ≤2.5m 室外) |
| 六轴传感器 | MPU6050 (计步精度 ≥90%) |
| 电池监测 | INA226 (电压精度 ±0.1%) |
| NB-IoT 模组 | QS-100 (全网通) |
| 电池 | 5V 2000mAh 锂电池 |
| 稳压 | LD1117-3.3V LDO |

### 机械设计

- **储粮机构**：漏斗形储粮仓 + SG90 舵机阀门，PWM 控制单次出粮量
- **供水机构**：5V 舵机 + 硅胶软管 + Water Sensor 水位反馈，自动补水闭环
- **外壳**：雪弗板手工制作，预留传感器开窗和通风散热孔
- **穿戴设备**：适合中小型犬佩戴

---

## 软件设计

### 技术栈

```
┌─────────────────────────────────────────────┐
│  移动端     │ ArkTS + ArkUI (HarmonyOS)       │
├─────────────────────────────────────────────┤
│  云服务     │ Spring Boot 3 + Java 17         │
├─────────────────────────────────────────────┤
│  消息队列   │ EMQX Broker (MQTT)              │
├─────────────────────────────────────────────┤
│  缓存/知识库│ Redis (倒排索引 + 向量存储)       │
├─────────────────────────────────────────────┤
│  对象存储   │ MinIO                           │
├─────────────────────────────────────────────┤
│  AI 推理    │ ONNX Runtime                    │
├─────────────────────────────────────────────┤
│  向量嵌入   │ DashScope text-embedding-v4      │
├─────────────────────────────────────────────┤
│  大语言模型 │ DeepSeek API                     │
├─────────────────────────────────────────────┤
│  嵌入式 OS  │ OpenHarmony + Huawei LiteOS     │
└─────────────────────────────────────────────┘
```

### 移动端功能模块

| 模块 | 功能 |
|------|------|
| 🏠 **首页看板** | 温湿度、光照、烟雾、粮水余量、GPS、步数、电池、宠物状态实时展示 |
| 🎮 **设备控制** | 手动/自动双模式、阈值规则编辑、宠物状态联动策略 |
| 🤖 **AI 对话** | DeepSeek 流式输出、多会话历史、RAG 检索详情、Function Calling |
| 📚 **知识库** | 文档上传、切片管理、Embedding 可视化、倒排索引、检索测试 |
| 🐕 **宠物档案** | 图片上传、狗脸特征入库、档案维护、识别结果联动 |

### 云端服务模块

| 模块 | 核心类/服务 | 功能 |
|------|------------|------|
| MQTT 接入 | `MqttEnvSubscriber`, `MqttFeedSubscriber` | 订阅环境/粮水/穿戴数据 Topic |
| 设备控制 | `DeviceControlService` | 手动/自动/阈值规则三种控制模式 |
| 视觉推理 | `DogDetectorService`, `DogFaceService` | YOLO 状态检测 + 狗脸特征提取 |
| 知识库 | `KnowledgeBaseService` | 文档解析、切片、索引、混合检索 |
| AI 对话 | `AiController` | DeepSeek 流式对话 + RAG + Function Calling |
| 宠物管理 | `PetStateService`, `MultiDogTrackerService` | 多宠识别、跨帧聚合、状态标注 |

---

## AI 能力

### 视觉识别管线

```
摄像头采集 → YOLO 目标检测 → 宠物状态分类
                           → 狗脸区域定位 → MobileNet+ArcFace → 128维 Embedding
                                                                      │
                                                         ┌────────────┘
                                                         ▼
                                              Redis 宠物特征库 (1:N 余弦匹配)
                                                         │
                                                         ▼
                                                   身份确认 → 触发联动规则
```

### AI 对话管线

```
用户消息
    │
    ├─ 注入实时业务上下文 (环境/步数/睡眠/档案)
    ├─ RAG 混合检索 (倒排索引 + 向量相似度)
    │    └─ Top-K 知识片段
    ├─ 组装 System Prompt
    │
    ▼
DeepSeek API (流式)
    │
    ├─ 通用问答 (宠物喂养/健康)
    ├─ Function Calling → get_current_environment()
    │                     get_pet_profile()
    │                     get_step_history()
    │                     get_sleep_summary()
    │                     get_device_state()
    │                     set_device_control()
    │
    ▼
移动端流式展示 + 可执行建议
```

---

## 技术指标

| 指标类别 | 技术参数 |
|----------|----------|
| **主控芯片（网关）** | RK2206 (ARM Cortex-A7, 1.2GHz) |
| **主控芯片（从机/穿戴）** | STM32F407VET6 (ARM Cortex-M4, 168MHz) |
| **操作系统** | 网关：OpenHarmony OS；从机：Huawei LiteOS |
| **LoRa 通信** | DX LCC68, 433MHz, 空旷距离 ≥1km |
| **WiFi 通信** | MQTT 协议, 自建 EMQX 云平台 |
| **NB-IoT 通信** | QS-100 模组, 全网通, 直连云平台 |
| **温湿度** | DHT22, ±0.5°C / ±2%RH |
| **烟雾检测** | MQ-2 |
| **光照检测** | BH1750 |
| **水位检测** | Water Sensor, 精度 ±2mm |
| **重量检测** | HX711 + 称重传感器, 精度 ±1g |
| **GPS 定位** | ATGM336H, 精度 ≤2.5m (室外) |
| **运动检测** | MPU6050 六轴传感器, 计步精度 ≥90% |
| **电池监测** | INA226, 电压精度 ±0.1% |
| **AI 视觉模型** | YOLO (640×640) + MobileNet+ArcFace (224×224, 128维), 余弦阈值 0.35 |
| **大语言模型** | DeepSeek API + RAG 检索增强, 支持 PDF/Word/TXT |
| **执行机构** | SG90 舵机（喂粮）、5V 风扇、5V 加湿器、5V 水泵 |
| **软件平台** | 鸿蒙 HarmonyOS 移动端, 手动/自动双模式 |
| **供电方式** | 网关/从机：5V DC；穿戴：5V 2000mAh 锂电池 |

---

## 应用场景

### 🏠 日常无人看护
主人外出上班或短期出差时，系统自动监测宠物环境温湿度、光照强度、烟雾浓度等指标，按设定阈值自动开启风扇降温、遮光帘遮阳、加湿器调湿。同时根据水位和粮重自动补充食物和水源。

### 🐕 多宠精准管理
通过 **YOLO 视觉识别 + 狗脸特征向量匹配**，精准区分不同宠物个体，记录每只宠物的进食频次、活动步数、活动轨迹，实现"一宠一档"的个性化健康管理与科学喂养。

### 🏃 户外活动追踪
宠物佩戴穿戴设备外出时，**GPS 实时定位 + NB-IoT 通信**确保主人随时掌握宠物位置，MPU6050 计步记录运动量，INA226 电池电压监测预警低电量。

### 📚 科学喂养咨询
用户可上传宠物品种、年龄、体重等专业饲养文档，系统利用 **RAG 检索增强 + DeepSeek 大模型**，提供专业化的科学喂养建议与健康咨询。

---

## 性能测试

| 测试项目 | 测试结果 |
|----------|----------|
| 🔗 **LoRa 通信距离** | 空旷环境实测 **1.2km** (SF=9) |
| 🌡️ **温湿度精度** | 温度偏差 ≤ ±5°C (与标准温湿度计对比) |
| ⚖️ **重量传感器** | 0-5000g 量程, 线性度 **R²=0.998**, 重复性误差 ≤ ±2g |
| 💧 **水位传感器** | 2-40cm 量程, 误差 ≤ ±2mm |
| ⚡ **端到端响应延迟** | APP 喂粮 → 舵机动作: 平均 **0.8s** |
| 🤖 **AI 视觉推理** | YOLO ONNX 推理 (640×640) + 狗脸 Embedding (224×224) 完整链路 |
| 🔋 **穿戴设备续航** | **~28 小时** (1次/分钟 GPS + 20Hz MPU6050 + 5分钟/NB-IoT, 2000mAh) |
| 🛡️ **系统稳定性** | 网关+从机连续运行 **72 小时** 无死机、无数据丢失 |
| 🔄 **MQTT 断连恢复** | WiFi 断开后 **10s 内**自动重连 |

---

## 快速开始

### 环境要求

- **嵌入式端**
  - OpenHarmony SDK (RK2206)
  - Huawei LiteOS SDK (STM32F407)
  - Keil MDK / STM32CubeIDE
- **云端**
  - JDK 17+
  - Spring Boot 3.x
  - Redis 7.x
  - MinIO
  - EMQX 5.x
  - ONNX Runtime 1.x
- **移动端**
  - DevEco Studio
  - HarmonyOS SDK (API 9+)

### 硬件连接

```
┌─────────────┐    LoRa (433MHz)    ┌──────────────┐
│  RK2206 网关 │ ◄──────────────────► │ STM32F407 从机│
│              │                      │  (喂粮控制)    │
└──────┬───────┘                      └──────────────┘
       │ WiFi
       ▼
┌─────────────┐
│  EMQX Cloud  │
└──────┬───────┘
       │
┌──────▼───────┐    NB-IoT           ┌──────────────┐
│ Spring Boot  │ ◄────────────────── │ STM32F407 从机│
│  后端服务     │                      │  (穿戴设备)    │
└──────────────┘                      └──────────────┘
```

### 配置文件

```yaml
# application.yml
spring:
  redis:
    host: ${REDIS_HOST}
    port: 6379
  mqtt:
    broker: ${EMQX_BROKER_URL}
    client-id: cloudpet-lingxi-server

deepseek:
  api-key: ${DEEPSEEK_API_KEY}
  model: deepseek-chat

dashscope:
  api-key: ${DASHSCOPE_API_KEY}
  embedding-model: text-embedding-v4

minio:
  endpoint: ${MINIO_ENDPOINT}
  access-key: ${MINIO_ACCESS_KEY}
  secret-key: ${MINIO_SECRET_KEY}
```

---

## 项目结构

```
CloudPet-Lingxi/
├── hardware/                    # 硬件设计文件
│   ├── gateway/                 # RK2206 网关原理图 & PCB
│   ├── feed_slave/              # 喂粮从机原理图 & PCB
│   └── wearable/                # 穿戴从机原理图 & PCB
├── firmware/                    # 嵌入式固件
│   ├── rk2206_gateway/          # OpenHarmony 网关固件
│   ├── feed_slave/              # LiteOS 喂粮从机固件
│   └── wearable/                # LiteOS 穿戴从机固件
├── backend/                     # Spring Boot 后端服务
│   ├── src/main/java/...
│   ├── src/main/resources/
│   └── pom.xml
├── ai-service/                  # AI 推理服务
│   ├── models/                  # ONNX 模型文件
│   │   ├── yolo_dog_detector.onnx
│   │   └── dogface_embed.onnx
│   └── src/...
├── mobile/                      # 鸿蒙 ArkTS 移动端
│   └── src/main/ets/
└── docs/                        # 文档
    └── README.md
```

---

## 发展展望

1. **AI 模型边缘化部署** 🚀
   将 YOLO 和狗脸识别模型量化压缩（TensorRT / NCNN），部署至 RK2206 或更强算力的边缘设备，实现本地离线推理。

2. **多模态数据融合深化** 🧬
   通过注意力机制融合网络，挖掘环境数据与宠物行为的深度关联，实现异常行为预警和早期疾病识别。

3. **产品化与商业化** 🏭
   - IP67 级防水、更小型化穿戴设备
   - 宠物项圈一体化方案
   - 增加心率、血氧等生物传感器

4. **生态扩展** 🌐
   接入更多智能家居设备（空调、空气净化器），融入全屋智能生态；构建宠物社区与用户生态闭环。

5. **数据价值挖掘** 📊
   长期积累的宠物行为与健康数据可用于品种习性研究、宠物保险精算、宠物食品研发等领域。

---

## 参考文献

1. 华为技术有限公司. OpenHarmony 设备开发文档. https://docs.openharmony.cn
2. 华为技术有限公司. Huawei LiteOS 开发指南. https://gitee.com/LiteOS/LiteOS
3. 瑞芯微电子. RK2206 数据手册. 2024.
4. STMicroelectronics. STM32F407xx Reference Manual (RM0090). 2023.
5. Semtech. DX LCC68 Datasheet. 2022.
6. Quectel. QS-100 NB-IoT Module Hardware Design Manual. 2023.
7. InvenSense. MPU-6050 Product Specification. 2021.
8. Texas Instruments. INA226 Datasheet. 2022.
9. Ultralytics. YOLOv8 Documentation. https://docs.ultralytics.com
10. Schroff F, et al. *FaceNet: A Unified Embedding for Face Recognition and Clustering*. CVPR, 2015.
11. DeepSeek. DeepSeek API Documentation. https://platform.deepseek.com/api-docs
12. Microsoft. ONNX Runtime Documentation. https://onnxruntime.ai/docs
13. EMQX Team. EMQX Documentation. https://www.emqx.io/docs
14. Lewis P, et al. *Retrieval-Augmented Generation for Knowledge-Intensive NLP Tasks*. NeurIPS, 2020.
15. Sandler M, et al. *MobileNetV2: Inverted Residuals and Linear Bottlenecks*. CVPR, 2018.
16. 华为技术有限公司. HarmonyOS ArkUI 开发指南. https://developer.huawei.com
17. 刘火良, 杨森. *STM32 库开发实战指南（第 3 版）*. 机械工业出版社, 2023.

---

## 👥 贡献者

本项目为嵌入式 AI 物联网竞赛作品，由团队成员共同完成硬件设计、嵌入式开发、云端服务和移动端应用的完整研发。

---

<p align="center">
  <b>🐾 云宠灵犀 — 让每一次陪伴都充满智慧 🐾</b>
</p>
