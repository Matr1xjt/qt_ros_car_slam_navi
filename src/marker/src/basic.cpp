#include "basic.h"
#include "ui_basic.h"
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QGridLayout>
#include <QPainter>
#include <QPushButton>
#include <QFileDialog>
#include <QDebug>
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

  label_map = new QLabel(this);
  label_odom = new QLabel(this);
  label_map->setFixedSize(600,600);
  layout->addWidget(label_map);
  setLayout(layout);
  nh_map = new ros::NodeHandle;
  map_sub = nh_map->subscribe("/map",1,&Basic::mapCallback,this);
  odom_sub = nh_map->subscribe("/odom",1,&Basic::odomCallback,this);

  QPushButton * save_map_btn = new QPushButton("save",this);
  save_map_btn->setStyleSheet("font-size:20px; padding:8px");
  save_map_btn->move(width()-200,height()-80);
  connect(save_map_btn,&QPushButton::clicked,this,&Basic::saveMapImage);
  drawButton = new QPushButton("draw", this);
  drawButton->setGeometry(10, 10, 100, 30);
  connect(drawButton, &QPushButton::clicked, this, &Basic::toggleDrawing);
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

void Basic::updateMapWithDrawing()
{
  QPixmap pixmap = QPixmap::fromImage(map_image);
  QPainter painter(&pixmap);
  painter.setBrush(Qt::red);
  painter.drawEllipse(robot_pos_px,5,5);

  painter.setBrush(Qt::blue);
  for (const QPoint & p: usr_draw_points){
    painter.drawEllipse(p,3,3);
  }
  display_pixmap = pixmap;
  label_map->setPixmap(display_pixmap);
}

void Basic::saveMapImage()
{
  QString filename = QFileDialog::getSaveFileName(this, "save_map", "", "PNG_picture (*.png);;JPEG_picture (*.jpg)");
  if (!filename.isEmpty())
    display_pixmap.save(filename);
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

void Basic::mapCallback(const nav_msgs::OccupancyGridPtr &msg)
{
  resolution = msg->info.resolution;
  int width = msg->info.width;
  int height = msg->info.height;
  const std::vector<int8_t> &data = msg->data;

  QImage image(width,height, QImage::Format_BGR888);
  for(int y=0; y<height ;y++){
    for(int x = 0; x< width; x++){
      int i = x +(height-1-y)*height;
      int8_t value = data[i];
      QColor color;

      if(value == 0)
        color = Qt::white;
      else if(value == 100)
        color = Qt::black;
      else
        color = Qt::gray;

      image.setPixelColor(x,y,color);
    }
  }
  map_image = image.scaled(label_map->size(),Qt::KeepAspectRatio,Qt::FastTransformation);
  QPixmap pixmap = QPixmap::fromImage(map_image);
  QPainter painter(&pixmap);
  painter.setBrush(Qt::red);
  painter.drawEllipse(robot_pos_px, 5, 5);
  for (const QPoint &p :usr_draw_points){
    painter.setBrush(Qt::blue);
    painter.drawEllipse(p,3,3);
  }
  display_pixmap = pixmap;
  label_map->setPixmap(display_pixmap);


}

void Basic::odomCallback(const nav_msgs::OdometryPtr &msg)
{
  float x = msg->pose.pose.position.x;
  float y = msg->pose.pose.position.y;

  int px = x / resolution;
  int py = y / resolution;

  robot_pos_px = QPoint(px, map_image.height()-py);

  QPixmap pixmap = QPixmap::fromImage(map_image);
  QPainter painter(&pixmap);
  painter.setBrush(Qt::red);
  painter.drawEllipse(robot_pos_px, 5, 5);

  display_pixmap = pixmap;
  label_map->setPixmap(display_pixmap);
}

void Basic::toggleDrawing()
{
  drawing_enabled = !drawing_enabled;
  if(drawing_enabled){
    drawButton->setText("drawing");
    qDebug() << "drawing started";
  }
  else{
    drawButton->setText("draw");
    qDebug() << "draw closed";
  }
  update();
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

void Basic::mousePressEvent(QMouseEvent *event)
{
  if (!drawing_enabled) return;

  QPoint pos = event->pos();

  if (label_map->geometry().contains(pos)){
    QPoint relative_pos = label_map->mapFromParent(pos);
    usr_draw_points.push_back(relative_pos);
    updateMapWithDrawing();
  }
}
