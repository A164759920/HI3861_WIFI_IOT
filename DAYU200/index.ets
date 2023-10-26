import router from '@ohos.router';
import prompt from '@ohos.prompt';
import { getDate } from './common/utils/time';
import { getFullYear } from './common/utils/time';
import { getSeconds } from './common/utils/time';
import { getTime } from './common/utils/time';
import { TextRow } from './common/optionItem';
import { ClickText } from './common/optionItem';

import wifi from '@ohos.wifi';
import socket from '@ohos.net.socket';
import http from '@ohos.net.http';

//tcp连接对象
let tcp = socket.constructTCPSocketInstance();

//目标地址和端口
let targetAddr = {
  address: '192.168.43.183', //要通信的地址，3861
  family: 1,
  port: 3861 //
}

let reconnectInterval = null;

enum MSG_CMD {
  Switch1_ON= "switch1:on", //移动
  Switch1_OFF= "switch1:off", //移动
  Switch2_ON = "switch2:on", //移动
  Switch2_OFF= "switch2:off", //移动

};
@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  private tabSrc: Resource[] = [$r("app.media.zhny1"), $r("app.media.zhny2"), $r("app.media.zhny3"), $r("app.media.zhny4")]
  private swiperController: SwiperController = new SwiperController()
  @State currentDate: string = getFullYear() + getDate()
  @State currentTime: string = getTime() + getSeconds()
  @State temp: string = "22.60"
  @State humi: string = "32.82"
  //  @State mc1: string = "1"
  //  @State mc2: string = "1"
  //  @State rt1: string = "1"
  @State isNight: boolean = false
  quality: string = '优'
  @State message_send: string = '{"temp":32.27,"humi":32.27,"rt1":1,"mc1":1,"mc2":1}'
  @State message_recv: string = ''
  @State status: string = '未连接'
  @State switch1State :Number = 0
  @State switch2State :Number = 0
  @State switch1COLOR:number = Color.Red // 第一个文本框背景颜色
  @State switch2COLOR:number = Color.Red // 第二个文本框背景颜色

  public attendanceOnClick1ON(event: ClickEvent) {
    this.message_send = MSG_CMD.Switch1_ON
    this.tcpSend()
  }

  public attendanceOnClick1Off(event: ClickEvent) {
    this.message_send = MSG_CMD.Switch1_OFF
    this.tcpSend()
  }

  public attendanceOnClick2ON(event: ClickEvent) {
    this.message_send = MSG_CMD.Switch2_ON
    this.tcpSend()
  }

  public attendanceOnClick2Off(event: ClickEvent) {
    this.message_send = MSG_CMD.Switch2_OFF
    this.tcpSend()
  }

  onPageShow() {
    // this.tcpConnect()    //原来注释
    setInterval(() => {
      let date = new Date
      // 判断是否为夜晚
      this.isNight = date.getHours() >= 22 ? true : false
      // 获取当前时间
      this.currentTime = (date.getHours() < 10 ? '0' + date.getHours() : date.getHours()) + ':' + (date.getMinutes() < 10 ? '0' + date.getMinutes() : date.getMinutes()) + ':' + (date.getSeconds() < 10 ? '0' + date.getSeconds() : date.getSeconds())
      // 判断是否为新的一天
      if (date.getHours() == 0) {
        this.currentDate = date.getFullYear() + '年' + (date.getMonth() + 1) + '月' + date.getDate() + '日'
      }
    }, 1000)
    setInterval(()=>{
      let httpRequest = http.createHttp();
      httpRequest.on('headersReceive', (header) => {
        // console.info('header: ' + JSON.stringify(header));
      });
      httpRequest.request(
        "http://api.heclouds.com/devices/1059893029/datapoints?datastream_id=humi,temperature&limit=1",
        {
          method:http.RequestMethod.GET,
          header:{
            'Authorization':"version=2018-10-31&res=products%2F583419&et=1698324774&method=md5&sign=JX8lEWvK7DpOnz%2FOS5uNrQ%3D%3D"
          }
        },(err,data)=> {
        if (!err) {
          let res =  JSON.stringify(data.result);
          // console.info(res)
          res = JSON.parse(res)
          console.info("==========================")
          console.info(res)
          const regex = /"value":(\d+)/g;
          let match;
          const values = [];

          while ((match = regex.exec(res)) !== null) {
            values.push(match[1]);
          }
          this.temp = values[0];
          this.humi = values[1];

        } else {
          console.info('error:' + JSON.stringify(err));
          httpRequest.off('headersReceive');
          httpRequest.destroy();
        }
      }
      )
    },2000)
  }

  aboutToAppear() {
    this.tcpConnect()
  }

  aboutToDisappear() {
    // this.tcpClose()
  }
  //本地地址和端口
  localAddr = {
    address: this.resolveIP(wifi.getIpInfo().ipAddress)
  }

  //解析本地IP,需要声明ohos.permission.GET_WIFI_INFO权限
  resolveIP(ip) {
    if (ip < 0 || ip > 0xFFFFFFFF) {
      throw ("The number is not normal!");
    }
    return (ip >>> 24) + "." + (ip >> 16 & 0xFF) + "." + (ip >> 8 & 0xFF) + "." + (ip & 0xFF);
  }
  //解析ArrayBuffer
  resolveArrayBuffer(message: ArrayBuffer): string {
    if (message instanceof ArrayBuffer) {
      let dataView = new DataView(message)
      console.info(`length ${dataView.byteLength}`)
      let str = ""
      for (let i = 0;i < dataView.byteLength; ++i) {
        let c = String.fromCharCode(dataView.getUint8(i))
        if (c !== "\n") {
          str += c
        }
      }
      console.info(`message aray buffer:${str}`)
      return str;
    }
  }
  //初始化
  tcpInit() {
    tcp.on('connect', () => {
      this.status = '已连接'
      console.info("on tcp connect success");
    });
    // 新添加的代码
    tcp.on('error', () => {
      reconnectInterval = setInterval(()=>{
        this.tcpConnect();
      },1000);

    });
    tcp.on('message', value => {
      // this.message_recv = this.resolveArrayBuffer(value.message)
      let _rawData =  this.resolveArrayBuffer(value.message)
      console.info("===========message=========")
      console.info(_rawData)
      console.info("===========message=========")


      // else
      // {
      // // 开关帧
      //
      //
      // }



      // var temp = "temp";
      // var humi = "humi";
      // prompt.showToast({ message: this.message_recv })
      // if (this.message_recv.includes(temp)) {
      //   console.log("temp" + this.message_recv)
      //   prompt.showToast({ message: this.message_recv })
      //   this.temp = this.message_recv.slice(5, 10);
      //   console.log("==收到消息温度" + this.message_recv.slice(5, 10))
      // }
      // if (this.message_recv.includes(humi)) {
      //   console.log("humi" + this.message_recv)
      //   prompt.showToast({ message: this.message_recv })
      //   this.humi = this.message_recv.slice(-5);
      //   console.log("==收到消息湿度" + this.message_recv.slice(-5))
      // }
      // if (this.message_recv.includes("mc1")) {
      //   let mc1 = this.message_recv.slice(4, 5);
      //
      //   //        this.mc1 = this.message_recv.slice(4, 5);
      //   console.log("==收到消息门禁1" + this.message_recv.slice(4, 5))
      // }
      // if (this.message_recv.includes("mc2")) {
      //
      //   //        this.mc2 = this.message_recv.slice(4, 5);
      //   console.log("==收到消息门禁2" + this.message_recv.slice(4, 5))
      // }
      // if (this.message_recv.includes("rt1")) {
      //
      //   //        this.rt1 = this.message_recv.slice(4, 5);
      //   console.log("==收到消息有人经过" + this.message_recv.slice(4, 5))
      // }

      // console.log("==收到消息:" + this.message_recv)


    });
    tcp.on('close', () => {
      console.info(" ==收到 on tcp close success");
    });

    //bind本地地址
    tcp.bind({ address: this.localAddr.address, port: 3861, family: 1 })
      .then(() => {
        console.info(` ==收到 bind tcp success`);
      }).catch(err => {
      console.info(` ==收到 bind tcp failed ${err}`);
      return
    });
  }
  //tcp 连接
  tcpConnect() {
    tcp.getState()
      .then((data) => {
        console.info(`====${JSON.stringify(data)}`)
        if (data.isClose) {
          this.tcpInit()
        }

        //开始连接
        tcp.connect(
          {
            address: { address: targetAddr.address, port: 3861, family: 1 }, timeout: 6000
          }
        ).then(() => {
          console.info(` ==收到 connect success`);

        }).catch((error) => {
          console.info(` ==收到 connect failed ${JSON.stringify(error)}`);
        })

      })
  }

  tcpSend() {
    //查看状态
    tcp.getState().then((data) => {
      console.info(`====${JSON.stringify(data)}`)
      //已连接,就发送数据
      if (data.isConnected) {
        //发送消息
        tcp.send(
          { data: this.message_send, }
        ).then(() => {
          prompt.showToast({ message: "数据发送成功" + this.message_send })
           let _rawData = this.message_send
          if (_rawData=="switch1:on" ) {
            // 处理 switch1 打开的逻辑
            console.info("命中1")
            this.switch1COLOR = Color.Green
          }
          // this.switch1State = 1;
          if (_rawData =="switch1:off" ) {
            // this.switch1State = 0;
            console.info("命中2")
            this.switch1COLOR = Color.Red
          }
          console.log("switch1 关闭");
          if (_rawData.includes("switch2:on")) {
            // 处理 switch2 打开的逻辑
            this.switch2State = 1;
            this.switch2COLOR = Color.Green
            console.info("命中3")
            console.log("switch2 打开");
          }
          if (_rawData.includes("switch2:off")) {
            // 处理 switch2 关闭的逻辑
            this.switch2State = 0;
            console.info("命中4")
            this.switch2COLOR = Color.Red
            console.log("switch2 关闭");
          }
        }).catch((error) => {
          prompt.showToast({ message: `send failed ${JSON.stringify(error)}` })

        })
      } else {
        prompt.showToast({ message: "not connect" })

      }
    })
  }

  tcpClose() {
    tcp.close().then(() => {
      this.status = '已断开'
      console.info(`tcp.close success`)
    }).catch((err) => {
      console.info(`tcp.close error:${JSON.stringify(err)}`)
    })
    tcp.off('close');
    tcp.off('message');
    tcp.off('connect');
  }

  build() {
    Row() {


      Column() {
        //温度，湿度，光照强度，土壤湿度，
        Text("智慧农业系统")
          .fontSize(36)
          .fontWeight(FontWeight.Bold)
        //当地温度
        Column() {
          Row() {
            if (this.isNight) {
              Image($r('app.media.moon'))
                .width(100)
                .height(100)
                .objectFit(ImageFit.None)
            } else {
              Image($r('app.media.sun'))
                .width(100)
                .height(100)
                .objectFit(ImageFit.None)
            }
            Text(this.currentDate + '\n' + this.currentTime)
              .fontSize(24)
              .fontColor(Color.White)
          }
          TextRow({ text: '温度', value: this.temp + '℃' })
          TextRow({ text: '空气质量', value: this.quality })
          TextRow({ text: '空气湿度', value: this.humi + "%RH" })
        }
        .onAppear(() => {
          this.tcpConnect()    //原来注释
        })
        .onClick(() => {
          // this.tcpConnect()    //原来注释
          // prompt.showToast({ message: "啊啊啊啊啊" })
          prompt.showToast({ message:this.message_recv })
          // this.tcpSend()
        })
        .width("90%")
        .borderRadius(30)
        .backgroundColor($r('app.color.blue_link'))
        .margin(20)
        //视频监控
        Swiper(this.swiperController) {
          ForEach(this.tabSrc, (item) => {
            Image(item).width('90%').height("30%").backgroundColor(0xAFEEEE)
          },)
        }
        .cachedCount(2)
        .index(1)
        .autoPlay(true)
        .interval(4000)
        .indicator(true) // 默认开启指示点
        .loop(true) // 默认开启循环播放
        .duration(1000)
        .vertical(false) // 默认横向切换
        .itemSpace(0)
        .curve(Curve.Linear) // 动画曲线
        .onChange((index: number) => {
          console.info(index.toString())
        })
        .margin(10)
        //控制区域
        Flex({ wrap: FlexWrap.Wrap, direction: FlexDirection.Row }) {
          ClickText({ context: "通风", itemOnClick: this.attendanceOnClick1ON.bind(this) })
          ClickText({ context: "浇水", itemOnClick: this.attendanceOnClick1Off.bind(this) })
          ClickText({ context: "施肥", itemOnClick: this.attendanceOnClick2ON.bind(this) })
          ClickText({ context: "补光", width: "100%", itemOnClick: this.attendanceOnClick2Off.bind(this) })
        }
        Row() {

            Text("[1]")
              .backgroundColor(this.switch1COLOR)
              .width(50)
              .height(50)

              .borderRadius(50)
              .textAlign(1)
              .margin(10)
          Text("[2]")
            .backgroundColor(this.switch2COLOR)
            .width(50)
            .height(50)
            .borderRadius(50)
            .textAlign(1)
            .margin(10)
        }

        //控制区域，通风，浇水，补光，施肥
        //        this.message_send = MSG_CMD.MOVE_GO
        //        this.tcpSend()

      }
      .width('100%')
    }
    .height('100%')
  }
}

