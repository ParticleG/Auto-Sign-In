#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <atomic>
#include "boost/asio.hpp"

namespace stea
{
	namespace asio::httpRequests
	{
		/// need boost-asio

		/// 函数声明
		std::string anyRequest(std::string method, std::string path, std::string host, std::string header, std::string body, std::string returnType, int startPos, int endPos);
		std::string postRequest(std::string path, std::string host, std::string header, std::string body, std::string returnType, int startPos, int endPos);
		std::string getRequest(std::string path, std::string host, std::string header, std::string returnType, int startPos, int endPos);
		
        /// 任意HTTP请求定义
		std::string anyRequest(std::string method, std::string path, std::string host, std::string header, std::string body, std::string returnType, int startPos, int endPos)
		{
			long length = body.length();
			if((returnType != "CODE")&&(returnType != "HEADER")&&(returnType != "BODY"))
			{
				throw "UNSUPPORTED_RESPONSE_TYPE";
			}
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
			request_stream << method << " " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << host << "\r\n";
			if(method == "POST")
			{
				request_stream << "Content-Length: " << length << "\r\n";
			}
			request_stream << header << "\r\n";
			request_stream << body;

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
				std::string responseCode;
				responseCode = std::to_string(status_code);
				return responseCode;
			}
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				throw "INVALID_RESPONSE";                                                                  ///Throw string
			}
			if (status_code >= 400 )
			{
				throw status_code;                                                                         ///Throw unsigned int
			}

			// 读取应答头部，遇到空行后停止
			boost::asio::read_until(socket, response, "\r\n\r\n");

			if (returnType == "HEADER")
			{
				std::istream is(&response);
				is.unsetf(std::ios_base::skipws);
				std::string responseHeader;
				responseHeader.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
				return responseHeader;
			}

			// 去除应答头部
			std::string headerBuffer;
			int len = 0;
			while (std::getline(response_stream, headerBuffer) && headerBuffer != "\r")
			{
				if (headerBuffer.find("Content-Length: ") == 0)
				{
					std::stringstream stream;
					stream << headerBuffer.substr(16);
					stream >> len;
				}
			}

			long size = response.size();

			if (size > 0)
			{
				// ... do nothing ...
			}

			boost::system::error_code error;  // 读取错误
			
			// 循环读取数据流，直到文件结束
			while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
			{
				// 获取应答长度
				size = response.size();
				if (len != 0)
				{
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
				std::string responseBody;
				responseBody.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
				responseBody = responseBody.substr(startPos, responseBody.length() - endPos);
				return responseBody;
			}
			return "UNEXPECTED_END";
		}
		/// POST请求定义
		std::string postRequest(std::string path, std::string host, std::string header, std::string body, std::string returnType, int startPos, int endPos)
		{
			std::string response;
			try
			{
				response = anyRequest("POST", path, host, header, body, returnType, startPos, endPos);
			}
			catch(std::string errorString)
			{
				throw errorString;
			}
			catch(unsigned int errorInt)
			{
				throw errorInt;
			}
			return response;
		}
		/// GET请求定义
		std::string getRequest(std::string path, std::string host, std::string header, std::string returnType, int startPos, int endPos)
		{
			std::string response;
			try
			{
				response = anyRequest("GET", path, host, header, "", returnType, startPos, endPos);
			}
			catch(std::string errorString)
			{
				throw errorString;
			}
			catch(unsigned int errorInt)
			{
				throw errorInt;
			}
			return response;
		}
	}
}
