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
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Odometry.h>
#include <QPushButton>
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
  void updateMapWithDrawing();
  void saveMapImage();
private slots:
  void updateLabel();
  void handleNewMessage(QString msg);
  static void rosMsgCallback(const std_msgs::String::ConstPtr &msg,Basic *context);
  void rgbCallback(const sensor_msgs::ImageConstPtr &msg);
  void depthCallback(const sensor_msgs::ImageConstPtr &msg);
  void mapCallback(const nav_msgs::OccupancyGridPtr &msg);
  void odomCallback(const nav_msgs::OdometryPtr &msg);
  void toggleDrawing();
private:
  Ui::Basic *ui;
  ros::NodeHandle *nh;
  ros::NodeHandle *nh_1;
  ros::NodeHandle *nh_twist;
  ros::NodeHandle *nh_image;
  ros::NodeHandle *nh_map;
  ros::Subscriber sub;
  ros::Publisher cmd_pub;
  ros::Publisher twist_pub;
  ros::Subscriber map_sub;
  ros::Subscriber odom_sub;
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
  QLabel *label_map;
  QLabel *label_odom;
  float resolution;
  QPoint robot_pos_px;
  QImage map_image;
  QPixmap display_pixmap;
  QVector<QPoint> usr_draw_points;
  bool drawing_enabled =false;
  QPushButton *drawButton;
  // QWidget interface
protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
};

#endif // BASIC_H
