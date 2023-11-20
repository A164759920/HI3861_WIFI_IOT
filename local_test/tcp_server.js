const net = require("net");

// 创建TCP服务器
const server = net.createServer((socket) => {
  // 当有新的连接时触发
  console.log("Client connected");

  // 监听数据接收事件
  socket.on("data", (data) => {
    // 处理接收到的数据
    console.log("Received data:", data.toString());

    // 发送响应数据
    socket.write("Server response");
  });

  // 监听连接关闭事件
  socket.on("close", () => {
    // 当连接关闭时触发
    console.log("Client disconnected");
  });

  // 监听错误事件
  socket.on("error", (error) => {
    // 当出现错误时触发
    console.error("Error:", error);
  });
});

// 监听指定的端口和主机地址
const port = 3861;
const hostname = "192.168.43.72";
server.listen(port, hostname, () => {
  console.log(`Server running at ${hostname}:${port}`);
});
