# pika_batch_ingest

## Introduction
### Mock + SST
用于高效生成大规模、可配置的 Key-Value 数据，支持多线程、文件大小拆分、重复数据比例控制，并以 JSON 格式输出 SST 文件，用于 RocksDB / Pika 等后续 ingest。
#### 方案概述
1. 用户可指定目标数据总大小（支持单位：M、G），支持生成高达 GB 级文件（本地磁盘允许范围内）。
2. 支持多线程并发数据生成，提高数据生成速度。
3. 自动拆分输出文件，每个文件最大大小可配置。
4. 数据包含可控比例的重复 Key 模拟真实场景。

#### 关键模块
✅数据生成模块
1. 随机或重复 Key-Value 数据生成（支持键值前缀、时间戳）。
2. 用户可配置目标数据大小、条目平均大小、每文件最大大小。

✅ 多线程文件生成器
1. 基于线程池提交文件生成任务。
2. 支持线程安全文件名分配。

✅ 文件管理模块
1. 按配置写入 JSON 格式文件。
2. 自动生成唯一文件名，支持多线程环境。

### S3 + ingest 主从同步方案设计
基于 S3 实现 SST 文件 ingest，并在 Pika 集群内主从同步，保证数据一致性，简化架构部署。

#### 方案概述
1. 主节点完成 ingest 后，通过扩展 binlog 记录一条特殊的 INGEST_SST 操作日志，包含必要的 manifest 元信息（如 manifest 版本、SST 文件列表等）。
2. 从节点在主从同步链路上接收到该 binlog 日志后，解析出 SST 文件信息，通过 S3 下载并按 manifest 指定顺序执行 ingest，保证数据一致性。
3. 为简化调用和运维：
   - 在 pika_kv 新增 INGEST_S3 命令接口，用于接收 ingest 指令并触发 ingest 逻辑。
   - 开发一个 Python agent，轮询 S3，检测新 manifest 文件，并通过管理接口调用主节点的 INGEST_S3 命令驱动 ingest。

#### 关键模块
✅ Pika 控制层扩展
1. 在 string 命令模块（如 pika_kv.h/cpp）添加新命令类（如 IngestS3Cmd）。
2. 接收 ingest_s3 命令参数（manifest 版本、文件列表）。
3. 调用 S3 拉取、RocksDB ingestExtend。
4. 写入扩展 binlog 日志（记录 ingest 操作信息）。

✅ 主从 binlog 链路扩展
1. 在 binlog 写入模块中定义新 RecordType，支持记录 INGEST_SST。
2. 主从同步链路传递 ingest 日志，从节点解析并执行。。

✅ Python agent
1. 定期轮询 S3，检测新 manifest 文件。
2. 调用主节点管理口发起 ingest 命令。


## Todo List
- [ ] 生成 SST 文件并实现主从复制
  - [x] 模拟数据，覆盖 String 类型
  - [x] 生成 SST 文件
    - [ ] 多线程生成 SST 文件
    - [ ] 自动扫描生成目录
    - [ ] SST 文件读取工具（用于验证文件内容）
  - [ ] 文件上传S3
    - [ ] S3 上传工具或脚本
    - [ ] 上传状态记录（可生成 manifest 记录文件）
  - [ ] 文件导入到Pika
    - [ ] Binlog 扩展支持 ingest 类型
    - [ ] 实现统一 S3 ingest 调用函数
    - [ ] 实现主节点 ingest 文件逻辑
    - [ ] 从节点解析 ingest binlog 并执行 ingest
- [ ] 自动化：实现自动化流程
  - [x] Mock自动化
  - [x] Exchange自动化
- [ ] 测试：编写相关测试，确保功能的正确性
  - [x] Mock单元测试
  - [x] Exchange单元测试

## Usage

```shell
../../build.sh
cd ../../output && ./pika_batch_ingest
```

## 各组件实现

### mock
用于生成模拟数据文件。使用方式如下：

```bash
./mock.sh -n {size} -d {dict}
```
示例（生成大小为 10GB，生成数据放在kvdict文件夹下）：
```bash
./mock.sh -n 10G -d "kvdict"
```
-n: 指定生成文件的大小，例如 10G, 500M 等；
-d: 指定生成数据的目录，用于存放生成内容；

实现效果如下
![alt text](images/mock.png)

### exchange

注意使用的rocksdb是动态库，需要添加一下动态库的系统变量

```shell
 export LD_LIBRARY_PATH=/home/byseea/code/opensum/pika_batch_ingest/third/rocksdb:$LD_LIBRARY_PATH
 ```

用于将模拟数据文件转化为sst文件。使用方式如下：

```bash
./exchange -k {kv} -s {sst}
```
示例（kv数据是放在kvdict文件夹下的data_1.json，生成的sst数据是放在sst文件夹下的data_1.sst）：
```bash
./exchange -k "kvdict/data_1.json" -s "sst/data_1.sst"
```
-k: 指定kv数据的文件；
-s: 指定sst数据的文件；

实现效果如下
![alt text](images/sst.png)