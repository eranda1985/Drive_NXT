#include "ros/ros.h"
#include "std_msgs/String.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

//----------------------------------------------------------------------------
ros::Publisher publisher;
geometry_msgs::Twist base_msg;
geometry_msgs::Twist last_published;
boost::mutex publish_mutex;
ros::Timer timer;
bool done = false;
bool firsts = false;
bool firsts_2 = false;
bool ss = false;
bool startangle = false;
bool overAllDone = false;
double startangleVal;
double inputAngle;
double currAngle;
double angleVelocity;
double solidAngle;
ros::Time now1;
ros::Time now1_2;
double currPositionX;
double currPositionY;
double lastPositionX;
double lastPositionY;
double distanceTravelled;
double distance_x;
double distance_y;
geometry_msgs::Quaternion cur_odom_quat;
geometry_msgs::Quaternion start_odom_quat;
geometry_msgs::Quaternion des_odom_quat;

void odomCallback(nav_msgs::Odometry msg) 
{
	if (inputAngle == 0) 
	{
		done = 1;
	}
	if (!startangle) 
	{
		start_odom_quat = msg.pose.pose.orientation;
		
		startangle = 1;
	}
	
	des_odom_quat = tf::createQuaternionMsgFromYaw(inputAngle);
	cur_odom_quat = msg.pose.pose.orientation;

	
	if (ss || (distance_x == 0)) 
	{
		base_msg.linear.x = 0;
		base_msg.angular.z = 0;
		
		if (!firsts_2) {
		now1_2 = ros::Time::now();
		firsts_2 = 1;
		}
		ros::Time now2_2 = ros::Time::now();
		ros::Duration d2 = now2_2 - now1_2;
		if(d2.toSec()>3.0)
		{
		overAllDone = true;
		}
		
		//overAllDone = true;
	}
	
	if(!done)
	{
		base_msg.angular.z = angleVelocity;
	}
	else
	{
		if (!firsts)
		{
			currPositionX = msg.pose.pose.position.x;
			currPositionY = msg.pose.pose.position.y;
			firsts = 1;
		}
		lastPositionX = msg.pose.pose.position.x;
		lastPositionY = msg.pose.pose.position.y;
		distanceTravelled = sqrt(((lastPositionX - currPositionX)*(lastPositionX - currPositionX)) + ((lastPositionY - currPositionY)*(lastPositionY - currPositionY)));
		if(distanceTravelled > (distance_x - 0.20))
		{	
			ss = 1;
			base_msg.linear.x = 0.0;
			//overAllDone = true;
		}
		else
		{
			base_msg.linear.x = 0.04;
			base_msg.angular.z = 0;
			//double dangle2 = cur_odom_quat.z - start_odom_quat.z;
			
				/*if (dangle2 > des_odom_quat.z) 
				{
					base_msg.angular.z = -0.025;
				}
			
			
				if (dangle2 < des_odom_quat.z) 
				{
					base_msg.angular.z = 0.025;
				}*/
			
			
		}
	}

	ROS_INFO("start Pos: %f", currPositionX);
	ROS_INFO("end Pos: %f", lastPositionX);
	
	last_published = base_msg;
	double dangle;
	dangle = cur_odom_quat.z - start_odom_quat.z;
	ROS_INFO("current angle: %f", currAngle);
	ROS_INFO("angle turned: %f", dangle);
	if(inputAngle > 0)
	{	
		if (dangle >= des_odom_quat.z) 
		{
			done = 1;
		}
	}
	else
	{
		if (dangle <= des_odom_quat.z) 
		{
			done = 1;
		}
	}
	
}
void publish()
{
	
	//boost::mutex::scoped_lock lock(publish_mutex);
	if(overAllDone || ss)
	{
		last_published.linear.x = 0;
	}
	
	publisher.publish(last_published);
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "nxt_drive");
	ros::NodeHandle nh;
	angleVelocity = 0.45;
	distanceTravelled = 0.0;
	

	if(argv[1] == NULL)
	{
		distance_x = 0.0;
	}
	if(argv[2] == NULL)
	{
		distance_y = 0.0;
	}
	if(argv[3] == NULL)
	{
		inputAngle = 0.0;
	}
	distance_x = atof(argv[1]);
	distance_y = atof(argv[2]);
	inputAngle = atof(argv[3]);
	//inputAngle += 0.2;

	if(inputAngle<0)
	{
		angleVelocity = -0.45;
	}
	else
	{
		//inputAngle += 0.2;
	} 

	
	base_msg.linear.x = 0;
	base_msg.angular.z = 0;
	
	ros::Subscriber sub = nh.subscribe("odom",1000,odomCallback);
	publisher = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
	timer = nh.createTimer(ros::Duration(0.1),boost::bind(publish));

	while(nh.ok() && !overAllDone)
	{
		ros::spinOnce();
		//r.sleep();
	}
	last_published.linear.x = 0;
	
	return 1 ?overAllDone:0;
}



