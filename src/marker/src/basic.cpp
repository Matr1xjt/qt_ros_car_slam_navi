#include "basic.h"
#include "ui_basic.h"
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QGridLayout>
#
Basic::Basic(int argc, char**argv,QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Basic),
  msg_now("")
{
  ui->setupUi(this);
  ros::init(argc, argv,"qt_ros_gui");
  resize(1400, 1400);
  nh = new ros::NodeHandle;

  sub = nh ->subscribe<std_msgs::String>("/chatter",10, boost::bind(&Basic::rosMsgCallback,_1,this));

  rostimer = new QTimer(this);
  connect(rostimer, &QTimer::timeout,this,[=](){
    ros::spinOnce();
  });
  rostimer->start(30);


  setFocusPolicy(Qt::StrongFocus);

  nh_1 = new ros::NodeHandle;
  cmd_pub = nh_1->advertise<std_msgs::String>("/car/cmd",10);
  move_ui();

  nh_twist = new ros::NodeHandle;
  twist_pub = nh_twist->advertise<geometry_msgs::Twist>("cmd_vel",1);
  speed = 0.5;
  turn = 1.0;

  label_speed_show = new QLabel(this);
  QString text = QString("current_speed:%1,turn:%2").arg(speed).arg(turn);
  label_speed_show->setText(text);
  label_speed_show->move(width()/2-650,height()-400);
  label_speed_show->setStyleSheet("font-size:28px; color: green");

  nh_image = new ros::NodeHandle;
  it = new image_transport::ImageTransport(*nh_image);

  label_rgb = new QLabel(this);
  label_depth = new QLabel(this);
  label_rgb->setFixedSize(640,480);
  label_depth->setFixedSize(640,480);
  QWidget *image_widget = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(image_widget);
  layout->addWidget(label_rgb);
  layout->addWidget(label_depth);
  image_widget->setLayout(layout);
  image_widget->move(width()/2-650,height()/2);
  rgb_sub = it->subscribe("/camera/color/image_raw",1, &Basic::rgbCallback, this);
  depth_sub = it->subscribe("/camera/aligned_depth_to_color/image_raw",1,&Basic::depthCallback,this);
}

Basic::~Basic()
{
  delete ui;
}

void Basic::setArrowState(const QString dir, bool active)
{


  if (dir == "speed_up" || dir == "speed_down" || dir == "turn_up" || dir == "turn_down")
    move_ctl_speed(dir);
  else{
    if (active == true){
      msg.data = dir.toStdString();
      arrows[dir]->setStyleSheet("font-size :36px; color: red" );
      label_move->setText(dir);
    }
    else {
      msg.data = dir.toStdString();
      arrows[dir]->setStyleSheet("font-size :36px; color: gray" );
      label_move->setText("stop");
    }
    move_ctl(dir);
  }
}

void Basic::move_ui()
{
  QWidget *arrowWidget = new QWidget(this);
  QGridLayout * layout = new QGridLayout(arrowWidget);

  QStringList dirs = {"up","down","left","right","up_right","up_left","down_right","down_left","stop"};
  for(const QString& dir :dirs){
    QLabel *label = new  QLabel(this);
    //label->setText("↑");
    label->setStyleSheet("font-size :36px; color: gray");
    arrows[dir]=label;
  }
  arrows["up"]->setText("↑");
  arrows["up"]->setAlignment(Qt::AlignCenter);
  arrows["down"]->setText("↓");
  arrows["down"]->setAlignment(Qt::AlignCenter);
  arrows["left"]->setText("←");
  arrows["right"]->setText("→");//←↑→↓↖↗↘↙
  arrows["up_right"]->setText("↗");
  arrows["up_left"]->setText("↖");
  arrows["down_right"]->setText("↘");
  arrows["down_left"]->setText("↙");
  //  arrows["up"]->move(width()/2-500,height()-300);
  //  arrows["down"]->move(width()/2-500,height()-100);
  //  arrows["left"]->move(100, height()-200);
  //  arrows["right"]->move(300,height()-200);
  label_move  = new QLabel(this);
  //  label_move->move(width()/2-500,height()-210);
  label_move->setText("stop");
  label_move->setStyleSheet("font-size:28px; color: black");
  //  label_move->adjustSize();
  label_move->setAlignment(Qt::AlignCenter);
  label_move->setFixedSize(150, 50);
  layout->addWidget(arrows["up"],0,1);
  layout->addWidget(arrows["down"],2,1);
  layout->addWidget(arrows["right"],1,2);
  layout->addWidget(arrows["left"],1,0);
  layout->addWidget(arrows["up_right"],0,2);
  layout->addWidget(arrows["up_left"],0,0);
  layout->addWidget(arrows["down_right"],2,2);
  layout->addWidget(arrows["down_left"],2,0);
  layout->addWidget(label_move,1,1);

  layout->setSpacing(25);
  layout->setContentsMargins(20,20,20,20);
  arrowWidget->setLayout(layout);
  arrowWidget->setFixedSize(300,300);
  arrowWidget->move(width()/2-650,height()-350);
}

void Basic::move_ctl(QString dir)
{
  QMap<QString,QList<int>> moveBinding;
  moveBinding["up"]={1,0,0,0};
  moveBinding["down"]={-1,0,0,0};
  moveBinding["left"]={0,0,0,1};
  moveBinding["right"]={0,0,0,-1};
  moveBinding["up_left"]={1,0,0,1};
  moveBinding["up_right"]={1,0,0,-1};
  moveBinding["down_left"]={-1,0,0,-1};
  moveBinding["down_right"]={-1,-1,0,0};

  int x,y,z, th;
  x = moveBinding[dir][0];
  y = moveBinding[dir][1];
  z = moveBinding[dir][2];
  th = moveBinding[dir][3];

  twist.linear.x = x *speed;
  twist.linear.y = y *speed;
  twist.linear.z = z *speed;
  twist.angular.x = 0;
  twist.angular.y = 0;
  twist.angular.z = th*turn;
  twist_pub.publish(twist);
}

void Basic::move_ctl_speed(QString dir)
{
  QMap<QString,QList<double>> speedBinding;
  speedBinding["speed_up"] = {1.1,1};
  speedBinding["speed_down"] = {0.9,1};
  speedBinding["turn_up"]={1,1.1};
  speedBinding["turn_down"]={1,0.9};

  speed = speed * speedBinding[dir][0];
  turn = turn * speedBinding[dir][1];
  QString text = QString("current_speed:%1,turn:%2").arg(speed).arg(turn);
  label_speed_show->adjustSize();
  label_speed_show->setText(text);
}


void Basic::rosMsgCallback(const std_msgs::String::ConstPtr &msg, Basic *context){
  QString qmsg = QString::fromStdString(msg->data);
  QMetaObject::invokeMethod(context, "handleNewMessage",
                            Qt::QueuedConnection,
                            Q_ARG(QString, qmsg));
}

void Basic::rgbCallback(const sensor_msgs::ImageConstPtr &msg)
{
  try {
    cv::Mat rgb = cv_bridge::toCvShare(msg, "bgr8")->image;
    cv::cvtColor(rgb, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    label_rgb->setPixmap(QPixmap::fromImage(img).scaled(label_rgb->size(),Qt::KeepAspectRatio));
  } catch (cv_bridge::Exception &e) {
    ROS_ERROR("RGB transport error: %s",e.what());
  }
}

void Basic::depthCallback(const sensor_msgs::ImageConstPtr &msg)
{
  try {
      cv::Mat depth =cv_bridge::toCvShare(msg, "16UC1")->image;
      cv::Mat depth_vis;
      depth.convertTo(depth_vis,CV_8SC1,0.03);

      QImage img(depth_vis.data,depth_vis.cols,depth_vis.rows,depth_vis.step, QImage::Format_Grayscale8);
      label_depth->setPixmap(QPixmap::fromImage(img).scaled(label_depth->size(), Qt::KeepAspectRatio));
  } catch (cv_bridge::Exception &e) {
    ROS_ERROR("Depth transport error: %s",e.what());
  }
}


void Basic::handleNewMessage(QString msg){
  msg_now = msg;
  updateLabel();
}

void Basic::updateLabel(){
  ui->label->setText(msg_now);
}

void Basic::keyPressEvent(QKeyEvent *event){
  switch(event->key()){
  case Qt::Key_W:
    setArrowState("up",true);
    break;
  case Qt::Key_X:
    setArrowState("down",true);
    break;
  case Qt::Key_D:
    setArrowState("right",true);
    break;
  case Qt::Key_A:
    setArrowState("left",true);
    break;
  case Qt::Key_Q:
    setArrowState("up_left",true);
    break;
  case Qt::Key_E:
    setArrowState("up_right",true);
    break;
  case Qt::Key_Z:
    setArrowState("down_left",true);
    break;
  case Qt::Key_C:
    setArrowState("down_right",true);
    break;
  case Qt::Key_Up:
    setArrowState("speed_up",true);
    break;
  case Qt::Key_Down:
    setArrowState("speed_down",true);
    break;
  case Qt::Key_Right:
    setArrowState("turn_up",true);
    break;
  case Qt::Key_Left:
    setArrowState("turn_down",true);
    break;
  default:
    setArrowState("down_right",false);
    break;
  }
  cmd_pub.publish(msg);
  return ;
}

void Basic::keyReleaseEvent(QKeyEvent *event){
  switch(event->key()){
  case Qt::Key_W:
    setArrowState("up",false);
    break;
  case Qt::Key_X:
    setArrowState("down",false);
    break;
  case Qt::Key_D:
    setArrowState("right",false);
    break;
  case Qt::Key_A:
    setArrowState("left",false);
    break;
  case Qt::Key_Q:
    setArrowState("up_left",false);
    break;
  case Qt::Key_E:
    setArrowState("up_right",false);
    break;
  case Qt::Key_Z:
    setArrowState("down_left",false);
    break;
  case Qt::Key_C:
    setArrowState("down_right",false);
    break;
  case Qt::Key_Up:
    setArrowState("speed_up",false);
    break;
  case Qt::Key_Down:
    setArrowState("speed_down",false);
    break;
  case Qt::Key_Right:
    setArrowState("turn_up",false);
    break;
  case Qt::Key_Left:
    setArrowState("turn_down",false);
    break;
  default:
    msg.data = "Stop";
    break;
  }

  cmd_pub.publish(msg);
}
