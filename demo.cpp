#define _CRT_SECURE_NO_WARNINGS

#include "cqsdk/cqsdk.h"
#include "NetworkRequest.h"

//using namespace std;
//using boost::asio::ip::tcp;

// namespace cq::app 包含插件本身的生命周期事件和管理
// namespace cq::event 用于注册 QQ 相关的事件处理函数
// namespace cq::api 用于调用酷 Q 提供的接口
// namespace cq::logging 用于日志
// namespace cq::message 提供封装了的 Message 等类

// 插件入口，在静态成员初始化之后，app::on_initialize 事件发生之前被执行，用于配置 SDK 和注册事件回调
CQ_MAIN {
    cq::app::on_enable = [] {
        // cq::logging、cq::api、cq::dir 等命名空间下的函数只能在事件回调函数内部调用，而不能直接在 CQ_MAIN 中调用
        cq::logging::debug(u8"启用", u8"Auto_Sign_In_Plugin is running!");
    };

    cq::event::on_private_msg = [](const cq::PrivateMessageEvent &e) {
        cq::logging::debug(u8"消息", u8"收到私聊消息：" + e.message + u8"，发送者：" + std::to_string(e.user_id));

        //if (e.user_id != 1135989508) return;

        try {
            cq::api::send_private_msg(e.user_id, e.message); // echo 回去

            cq::api::send_msg(e.target, e.message); // 使用 e.target 指定发送目标

            // MessageSegment 类提供一些静态成员函数以快速构造消息段
            cq::Message msg = cq::MessageSegment::contact(cq::MessageSegment::ContactType::GROUP, 628669530);
            msg.send(e.target); // 使用 Message 类的 send 成员函数
        } catch (const cq::exception::ApiError &err) {
            // API 调用失败
            cq::logging::debug(u8"API", u8"调用失败，错误码：" + std::to_string(err.code));
        }

        e.block(); // 阻止事件继续传递给其它插件
    };

    cq::event::on_group_msg = [](const auto &e /* 使用 C++ 的 auto 关键字 */) {
		using namespace std;
		//int position;
		//正则匹配
    	regex regexSignInAttempt("#code=(\\d{4})&amp;token=(\\w{8}-\\w{4}-\\w{4}-\\w{4}-\\w{12})");
		regex regexLoginAttempt("#account=(\\d{11})&amp;password=(\\S{6,20})");
		smatch result;
		
		if(regex_match(e.raw_message,result,regexLoginAttempt)){
			string yibanUsername = result[1], yibanPassword = result[2], xAuthToken;
			xAuthToken = totalXAuthToken(yibanUsername, yibanPassword);

			cq::Message msg = u8"登录测试:";
			msg += u8"\n用户QQ：" + std::to_string(e.user_id);
			msg += u8"\n易班账号：" + yibanUsername; // Message 类可以进行加法运算
            msg += u8"\n易班密码：" + yibanPassword; // Message 类可以进行加法运算
			msg += u8"\n返回：" + xAuthToken; // Message 类可以进行加法运算
			
			cq::message::send(e.target, msg);
		}
		else if(regex_match(e.raw_message,result,regexSignInAttempt)){
			//smatch::iterator x;
			string signInCode, xAuthToken, statusCode;
			cq::Message msg = u8"签到测试"; // string 到 Message 自动转换
			signInCode = result[1];
			msg += u8"\n签到码：" + signInCode; // Message 类可以进行加法运算
			xAuthToken = result[2];
            msg += u8"\nX-Auth-Token：" + xAuthToken; // Message 类可以进行加法运算
			statusCode = totalValidationCode(signInCode, xAuthToken);
			msg += u8"\n返回：" + statusCode; // Message 类可以进行加法运算
            cq::message::send(e.target, msg); // 使用 message 命名空间的 send 函数
			
		}
        //const auto memlist = cq::api::get_group_member_list(e.group_id); // 获取数据接口
        //cq::Message msg = u8"检测到精确消息，RawMessage="; // string 到 Message 自动转换
        //msg += std::to_string(memlist.size()) + u8" 个成员"; // Message 类可以进行加法运算
        //msg += e.raw_message; // Message 类可以进行加法运算
        //cq::message::send(e.target, msg); // 使用 message 命名空间的 send 函数
    };
}

// 添加菜单项，需要同时在 <appid>.json 文件的 menu 字段添加相应的条目，function 字段为 menu_demo_1
CQ_MENU(menu_demo_1) {
    cq::logging::info(u8"菜单", u8"点击了示例菜单1");
    try {
        cq::api::send_private_msg(1135989508, "hello");
    } catch (const cq::exception::ApiError &) {
        cq::logging::warning(u8"菜单", u8"发送失败");
    }
}

#include <Windows.h>

// 不像 CQ_MAIN，CQ_MENU 可以多次调用来添加多个菜单
CQ_MENU(menu_demo_2) {
    cq::logging::info(u8"菜单", u8"点击了示例菜单2");
    MessageBoxW(nullptr, L"这是一个提示", L"提示", MB_OK | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
}
void exceptionTest(){
	int a = -1;
	throw a;
}
std::string totalValidationCode(std::string signInCode, std::string xAuthToken) {
	std::string imageBase64, validationCode;
	try{
		imageBase64 = GetValidationCode(signInCode,xAuthToken);
	}
	catch(unsigned int statusCode){
		return "INVALID_SIGNIN_ATTEMPT: STATUS_CODE = " + std::to_string(statusCode);
	}
	validationCode = SendValidationCode(imageBase64);
	return SignInRequest(validationCode, xAuthToken);
}
std::string SignInRequest(std::string validationCode, std::string xAuthToken) {
	std::string responseCode = "";
	std::string requestHost = "skl.hdu.edu.cn";
	std::string requestPath = "/api/checkIn/valid-code?code=" + validationCode;
	std::string requestHeaders = "Pragma: no-cache\r\n"
								 "Cache-Control: no-cache\r\n"
								 "Accept: application/json, text/plain, */*\r\n"
								 "X-Auth-Token: " + xAuthToken + "\r\n"
								 "User-Agent: Mozilla/5.0 (Linux; Android 8.1.0; ONEPLUS A6000 Build/OPM6.171019.030.H1; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/72.0.3626.121 Mobile Safari/537.36 yiban_android\r\n"
								 "Referer: http://skl.hdu.edu.cn/?type=2\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Accept-Language: zh-CN,en-US;q=0.9\r\n"
								 "Cookie: client=android\r\n"
								 "X-Requested-With: com.yiban.app\r\n"
								 "Connection: close\r\n";
	try{
		responseCode = GetRequest(requestHost, requestPath, requestHeaders, "CODE");
	}
	catch(unsigned int statusCode){
		if(statusCode == 401){
			return "INVALID_SIGNIN_CODE";
		}
		else if(statusCode == 400){
			return "INVALID_VALIDATION_CODE";
		}
	}
	return "SUCCESS";
}
std::string SendValidationCode(std::string imageBase64) {
	std::string responseBody = "";
	std::string requestHost = "www.damagou.top";
	std::string requestPath = "/apiv1/recognize.html";
	std::string requestBody = "{\"image\":\"" + imageBase64 + "\",\"userkey\":\"737a6f3402df4b196aa956566325a5fb\",\"type\":\"1002\"}\r\n";
	responseBody = PostRequest(requestHost, requestPath, requestBody);
	return responseBody;
}
std::string GetValidationCode(std::string signInCode, std::string xAuthToken) {
	std::string responseBody = "", imageBase64 = "";
	std::string requestHost = "skl.hdu.edu.cn";
	std::string requestPath = "/api/checkIn/code-check-in?code=" + signInCode + "&w=70&h=26";
	std::string requestHeaders = "Pragma: no-cache\r\n"
								 "Cache-Control: no-cache\r\n"
								 "Accept: application/json, text/plain, */*\r\n"
								 "X-Auth-Token: " + xAuthToken + "\r\n"
								 "Accept-Language: zh-CN,en-US;q=0.9\r\n"
								 "X-Requested-With: com.yiban.app\r\n"
								 "Connection: close\r\n";
	try{
		responseBody = GetRequest(requestHost, requestPath, requestHeaders, "BODY");
	}
	catch(unsigned int statusCode){
		throw statusCode;
	}
	const unsigned char *imageBlob = (unsigned char *)(responseBody.c_str());
	imageBase64 = cq::utils::base64::encode(imageBlob,responseBody.size());
	return imageBase64;
}
std::string totalXAuthToken(std::string yibanUsername, std::string yibanPassword) {
	std::string accessToken, authPath, xAuthToken;
	try{
		accessToken = GetAccessToken(yibanUsername, yibanPassword);
	}
	catch(std::string error){
		return "INVALID_LOGIN_ATTEMPT: CODE = " + error;
	}
	try{
		authPath = GetAuthURL(accessToken);
	}
	catch(std::string error){
		return error;
	}
	try{
		xAuthToken = GetXAuthToken(authPath);
	}
	catch(std::string error){
		return error;
	}
	return xAuthToken;
}
std::string GetAccessToken(std::string yibanUsername, std::string yibanPassword) {
	int pos;
	std::string responseBody = "";
	std::string requestHost = "mobile.yiban.cn";
	std::string requestPath = "/api/v2/passport/login?account=" + yibanUsername + "&passwd=" + yibanPassword + "&ct=2&app=1&v=4.6.2&apn=3gnet&identify=867520044297975&sig=316ab653300d5108&token=&device=OnePlus%3AONEPLUS+A6003&sversion=28";
	std::string requestHeaders = "Content-type:text/json\r\n"
								 "Authorization: Bearer\r\n"
								 "loginToken:\r\n"
								 "AppVersion: 4.6.2\r\n"
								 "Connection: close\r\n";
	responseBody = GetRequest(requestHost, requestPath, requestHeaders, "BODY");
	if ((pos = responseBody.find("access_token=")) != std::string::npos) {
		std::string accessToken = responseBody.substr(pos + 13);
		accessToken = accessToken.erase(32);
		return accessToken;
	}
	else if ((pos = responseBody.find("response")) != std::string::npos) {
		std::string responseCode = responseBody.substr(pos + 10);
		responseCode = responseCode.erase(3);
		throw responseCode;
	}
}
std::string GetAuthURL(std::string accessToken) {
	int pos;
	std::string responseBody = "";
	std::string requestHost = "f.yiban.cn";
	std::string requestPath = "/iapp/index?act=iapp319528&v=" + accessToken;
	std::string requestHeaders = "Content-type:text/json\r\n"
								 "Cache-Control: no-cache\r\n"
								 "Upgrade-Insecure-Requests: 1\r\n"
								 "Referer: https://oauth.yiban.cn/code/html?client_id=dc76b1d4eba657b0&redirect_uri=http://f.yiban.cn/iapp319528\r\n"
								 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Accept-Language: zh-CN,en-US;q=0.9\r\n"
								 "Cookie: client=android; _YB_OPEN_V2_0=r2130lIb0h1env80; yibanM_user_token=" + accessToken + "\r\n"
								 "X-Requested-With: com.yiban.app\r\n"
								 "Connection: close\r\n";
	responseBody = GetRequest(requestHost, requestPath, requestHeaders,"HEADER");
	if ((pos = responseBody.find("verify_request=")) != std::string::npos) {
		std::string authPath = responseBody.substr(pos);
		pos = authPath.find("\r\n");
		authPath = authPath.erase(pos);
		return authPath;
	}
	else {
		throw "AUTH_ERROR";
	}
}
std::string GetXAuthToken(std::string authPath) {
	int pos;
	std::string responseBody = "";
	std::string requestHost = "skl.hdu.edu.cn";
	std::string requestPath = "/api/login/yiban?" + authPath;
	std::string requestHeaders = "Connection: close\r\n";
	responseBody = GetRequest(requestHost, requestPath, requestHeaders, "HEADER");
	if ((pos = responseBody.find("X-Auth-Token:")) != std::string::npos) {
		std::string xAuthToken = responseBody.substr(pos + 14);
		pos = xAuthToken.find("\r\n");
		xAuthToken = xAuthToken.erase(pos);
		return xAuthToken;
	}
	else {
		throw "TOKEN_ERROR";
	}
}

/// POST请求
std::string PostRequest(std::string host, std::string path, std::string form){
	long length = form.length();

	// 声明Asio基础: io_service（任务调度机）
	boost::asio::io_service io_service;

	// 获取服务器终端列表
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, "http");
	boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

	// 尝试连接每一个终端，直到成功建立socket连接
	boost::asio::ip::tcp::socket socket(io_service);
	boost::asio::connect(socket, iter);

	// 构建网络请求头
	// 指定 "Connection: close" 在获取应答后断开连接，确保获文件全部数据。
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "POST " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: */*\r\n";
	//request_stream << "Content-Type:application/x-www-form-urlencoded\r\n";
	request_stream << "Content-Length: " << length << "\r\n";
	request_stream << "Connection: close\r\n\r\n"; // 注意这里是两个空行
	request_stream << form; //POST 发送的数据本身不包含多于空行

	// 发送请求
	boost::asio::write(socket, request);

	// 读取应答状态. 应答缓冲流 streambuf 会自动增长至完整的行
	// 该增长可以在构造缓冲流时通过设置最大值限制
	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n");

	// 检查应答是否OK.
	std::istream response_stream(&response);// 应答流
	std::string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	std::string status_message;
	std::getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		;
		//printf("无效响应\n");
	}
	if (status_code != 200)
	{
		;
		//printf("响应返回 status code %d\n", status_code);
	}

	// 读取应答头部，遇到空行后停止
	boost::asio::read_until(socket, response, "\r\n\r\n");

	// 显示应答头部
	std::string header;
	int len = 0;
	while (std::getline(response_stream, header) && header != "\r")
	{
		if (header.find("Content-Length: ") == 0) {
			std::stringstream stream;
			stream << header.substr(16);
			stream >> len;
		}
	}

	long size = response.size();

	if (size > 0) {
		// .... do nothing
	}

	// 循环读取数据流，直到文件结束
	boost::system::error_code error;
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
	{
		// 获取应答长度
		size = response.size();
		if (len != 0) {
			std::cout << size << "  Byte  " << (size * 100) / len << "%\n";
		}

	}
	if (error != boost::asio::error::eof)
	{
		throw boost::system::system_error(error);
	}

	//std::cout << size << " Byte 内容已下载完毕." << std::endl;

	// 将streambuf类型转换为string类型返回
	std::istream is(&response);
	is.unsetf(std::ios_base::skipws);
	std::string sz;
	sz.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
	sz = sz.substr(3, 4);
	// 返回转换后的字符串
	return sz;
}

/// GET请求
std::string GetRequest(std::string host, std::string path, std::string headers, std::string returnType){
	std::string responseCode = "", responseHeader = "", responseBody = "";
	// 声明Asio基础: io_service（任务调度机）
	boost::asio::io_service io_service;

	// 获取服务器终端列表
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, "http");
	boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

	// 尝试连接每一个终端，直到成功建立socket连接
	boost::asio::ip::tcp::socket socket(io_service);
	boost::asio::connect(socket, iter);

	// 构建网络请求头.
	// 指定 "Connection: close" 在获取应答后断开连接，确保获文件全部数据。
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "GET " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << headers << "\r\n";
	//request_stream << "Connection: close\r\n\r\n";

	// 发送请求
	boost::asio::write(socket, request);

	// 读取应答状态. 应答缓冲流 streambuf 会自动增长至完整的行
	// 该增长可以在构造缓冲流时通过设置最大值限制
	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n\r\n");

	// 检查应答是否OK.
	std::istream response_stream(&response);
	std::string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	if(returnType == "CODE")
	{
		responseCode = std::to_string(status_code);
	}
	std::string status_message;
	std::getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		return "INVALID_RESPONSE";
		//printf("响应无效\n");
	}
	if ((status_code != 200)&&(status_code != 302))
	{
		throw status_code;
	}

	// 读取应答头部，遇到空行后停止
	boost::asio::read_until(socket, response, "\r\n\r\n");

	if (returnType == "HEADER")
	{
		std::istream is(&response);
		is.unsetf(std::ios_base::skipws);
		responseHeader.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
	}


	// 去除应答头部
	std::string header;
	int len = 0;
	while (std::getline(response_stream, header) && header != "\r")
	{
		if (header.find("Content-Length: ") == 0) {
			std::stringstream stream;
			stream << header.substr(16);
			stream >> len;
		}
	}

	long size = response.size();

	if (size > 0) {
		// ... do nothing ...
	}

	boost::system::error_code error;  // 读取错误

	// 循环读取数据流，直到文件结束
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
	{
		// 获取应答长度
		size = response.size();
		if (len != 0) {
			;
			//std::cout << size << "  Byte  " << (size * 100) / len << "%" << std::endl;
		}
	}

	// 如果没有读到文件尾，抛出异常
	if (error != boost::asio::error::eof)
	{
		throw boost::system::system_error(error);
	}

	//std::cout << size << " Byte 内容已下载完毕." << std::endl;

	if (returnType == "BODY")
	{
		// 将streambuf类型转换为string类型，并返回
		std::istream is(&response);
		is.unsetf(std::ios_base::skipws);
		responseBody.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
		responseBody = responseBody.substr(5, responseBody.length() - 12);
	}
	
	return responseCode + responseHeader + responseBody;
}