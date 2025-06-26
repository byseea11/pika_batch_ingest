# pika_batch_ingest

## Introduction
场景：一个用户有 30GB 的数据需要写入 Pika。之前只能通过 Redis 命令逐条写入（如 `SET key value`）。通过这个项目，客户端可以先根据规则将这 30GB 数据生成 SST 文件，然后将这些文件注入到 Pika 的 RocksDB 中。对于模拟数据，暂时只需要覆盖 **String** 类型即可。

## Todo List
- [ ] 生成 SST 文件并实现主从复制
  - [x] 模拟数据，覆盖 String 类型
  - [ ] 生成 SST 文件
  - [ ] 执行 Compaction 操作
  - [ ] 上传到共享存储并通知 Pika 服务端
  - [ ] 在 Pika 服务端实现主从复制
- [ ] 自动化：实现自动化流程
- [ ] 测试：编写相关测试，确保功能的正确性
  - [x] Mock单元测试

## Usage

```shell
../../build.sh
cd ../../output && ./pika_batch_ingest
```
