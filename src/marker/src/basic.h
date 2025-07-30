#ifndef BASIC_H
#define BASIC_H

#include <QWidget>
#include <std_msgs/String.h>
#include <ros/ros.h>
#include <QKeyEvent>
#include<QLabel>
#include <geometry_msgs/Twist.h>
#include <QImage>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
namespace Ui {
class Basic;
}

class Basic : public QWidget
{
  Q_OBJECT

public:
  explicit Basic(int argc,char ** argv,QWidget *parent = nullptr);
  ~Basic();
  void setArrowState(const QString dir, bool active);
  void move_ui();
  void move_ctl(QString dir);
  void move_ctl_speed(QString dir);
private slots:
  void updateLabel();
  void handleNewMessage(QString msg);
  static void rosMsgCallback(const std_msgs::String::ConstPtr &msg,Basic *context);
  void rgbCallback(const sensor_msgs::ImageConstPtr &msg);
  void depthCallback(const sensor_msgs::ImageConstPtr &msg);
private:
  Ui::Basic *ui;
  ros::NodeHandle *nh;
  ros::NodeHandle *nh_1;
  ros::NodeHandle *nh_twist;
  ros::NodeHandle *nh_image;
  ros::Subscriber sub;
  ros::Publisher cmd_pub;
  ros::Publisher twist_pub;
  image_transport::ImageTransport *it;
  image_transport::Subscriber rgb_sub;
  image_transport::Subscriber depth_sub;
  QTimer *rostimer;
  QString msg_now;
  QMap<QString, QLabel*> arrows;
  std_msgs::String msg;
  QLabel * label_move;
  QLabel * label_speed_show;
  geometry_msgs::Twist twist;
  double speed ;
  double turn ;
  QLabel *label_rgb;
  QLabel *label_depth;
  // QWidget interface
protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // BASIC_H
