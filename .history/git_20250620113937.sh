#!/bin/bash

# 提交消息参数
if [ -z "$1" ]; then
  echo "请输入提交消息: "
  exit 1
fi

# 自动添加所有文件
echo "添加所有文件变更..."
git add .

# 提交代码
echo "提交代码：$1"
git commit -m "$1"

# 推送到远程仓库
echo "推送到远程仓库..."
git push

# 输出完成消息
echo "代码已成功提交并推送！"
