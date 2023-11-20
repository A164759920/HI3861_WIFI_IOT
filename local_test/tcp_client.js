const net = require("net");

// TCP服务器的主机和端口
const serverHost = "192.168.43.183";
const serverPort = 3861;

// 创建TCP客户端
let client;
let reconnectDelay = 1000; // 初始重连延迟为1秒

function createClient() {
  console.log(`正在连接服务器${serverHost}:${serverPort}`);
  client = net.createConnection({ host: serverHost, port: serverPort });

  // 连接成功时触发
  client.on("connect", () => {
    console.log("已连接到服务器");
    reconnectDelay = 1000; // 重置重连延迟为1秒

    // 向服务器发送数据
    setInterval(() => {
      client.write("Hello, server!");
    }, 2000);
  });

  // 收到服务器发送的数据时触发
  client.on("data", (data) => {
    console.log("收到服务器的响应：", data.toString());
  });

  // 连接断开时触发
  client.on("end", () => {
    console.log("与服务器的连接已断开");
    console.log("正在尝试重新连接...");
    setTimeout(createClient, reconnectDelay); // 使用当前重连延迟重新连接
    reconn  ectDelay *= 2; // 延迟时间翻倍，进行指数增长
  });

  // 连接错误时触发，在此处添加重连逻辑
  client.on("error", (err) => {
    console.error("连接出错：", err);
    console.log("正在尝试重新连接...");
    setTimeout(createClient, reconnectDelay); // 使用当前重连延迟重新连接
    reconnectDelay *= 2; // 延迟时间翻倍，进行指数增长
  });
}

createClient();
