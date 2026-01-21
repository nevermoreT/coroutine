#!/bin/bash

# Echo Server 测试脚本

set -e

PORT=8888
SERVER_PID=0

# 清理函数
cleanup() {
    if [ $SERVER_PID -ne 0 ]; then
        echo "停止服务器 (PID: $SERVER_PID)"
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

# 注册清理函数
trap cleanup EXIT

echo "=== 编译项目 ==="
make clean
make

echo ""
echo "=== 启动 Echo Server ==="
./echo_server $PORT &
SERVER_PID=$!

# 等待服务器启动
sleep 1

# 检查服务器是否运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "错误: 服务器启动失败"
    exit 1
fi

echo "服务器已启动 (PID: $SERVER_PID)"
echo ""

# 运行测试客户端
echo "=== 运行测试客户端 ==="
./test_client 127.0.0.1 $PORT

echo ""
echo "=== 测试完成 ==="
