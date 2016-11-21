#ifndef COPENNI_H
#define COPENNI_H

#include <XnCppWrapper.h>
#include <XnCyclicStackT.h>
#include <XnHashT.h>
#include <XnListT.h>
#include <iostream>
#include <map>
#include <vector>


using namespace xn;
using namespace std;

class COpenNI
{
public:
	COpenNI();
	~COpenNI();

	/*OpenNI���ڲ���ʼ������������*/
	bool Initial();

	/*����OpenNI��ȡKinect����*/
	bool Start();

	/*����OpenNI��ȡ��������*/
	bool UpdateData();

	void Stop();

	/*�õ�ɫ��ͼ���node*/
	ImageGenerator& getImageGenerator();

	/*�õ����ͼ���node*/
	DepthGenerator& getDepthGenerator();

	/*�õ������node*/
	UserGenerator& getUserGenerator();

	/*�õ��������Ƶ�node*/
	GestureGenerator& getGestureGenerator();

	/*�õ��ֲ���node*/
	HandsGenerator& getHandGenerator();

	DepthMetaData depth_metadata_;   //�������ͼ������
	ImageMetaData image_metadata_;   //���ز�ɫͼ������
	std::map<XnUserID, XnPoint3D> hand_points_;  //Ϊ�˴洢��ͬ�ֵ�ʵʱ������õ�
	std::map< XnUserID, vector<XnPoint3D> > hands_track_points_; //Ϊ�˻滭���治ͬ�ֲ��ĸ��ٹ켣���趨��

private:
	/*�ú����������������˴��󣬷��ؼٴ�����ȷ*/
	bool CheckError(const char* error);

	/*��ʾ���������Ļص�����*/
	static void XN_CALLBACK_TYPE CBNewUser(UserGenerator &generator, XnUserID user, void *p_cookie);

	/*��ʾ����У����ɵĻص�����*/
	static void XN_CALLBACK_TYPE CBCalibrationComplete(SkeletonCapability &skeleton,XnUserID user, XnCalibrationStatus calibration_error, void *p_cookie);

	/*��ʾĳ�����ƶ����Ѿ���ɼ��Ļص�����*/
	static void XN_CALLBACK_TYPE  CBGestureRecognized(xn::GestureGenerator &generator, const XnChar *strGesture,const XnPoint3D *pIDPosition, const XnPoint3D *pEndPosition,void *pCookie);

	/*��ʾ��⵽ĳ�����ƿ�ʼ�Ļص�����*/
	static void XN_CALLBACK_TYPE CBGestureProgress(xn::GestureGenerator &generator, const XnChar *strGesture,const XnPoint3D *pPosition, XnFloat fProgress, void *pCookie);

	/*�ֲ���ʼ�����Ļص�����*/
	static void XN_CALLBACK_TYPE HandCreate(HandsGenerator& rHands, XnUserID xUID, const XnPoint3D* pPosition,XnFloat fTime, void* pCookie);

	/*�ֲ���ʼ���µĻص�����*/
	static void XN_CALLBACK_TYPE HandUpdate(HandsGenerator& rHands, XnUserID xUID, const XnPoint3D* pPosition, XnFloat fTime,void* pCookie);

	/*�ֲ����ٵĻص�����*/
	static void XN_CALLBACK_TYPE HandDestroy(HandsGenerator& rHands, XnUserID xUID, XnFloat fTime, void* pCookie);

	XnStatus status_;
	Context context_;
	XnMapOutputMode xmode_;
	UserGenerator user_generator_;
	ImageGenerator  image_generator_;
	DepthGenerator  depth_generator_;
	GestureGenerator gesture_generator_;
	HandsGenerator  hand_generator_;
};

#endif // COPENNI_H

