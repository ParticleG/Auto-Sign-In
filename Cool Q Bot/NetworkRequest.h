#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include "boost/asio.hpp"

//using namespace std;

/// GET请求
std::string GetRequest(std::string host, std::string path, std::string headers, std::string returnType);

/// POST请求
std::string PostRequest(std::string host, std::string path, std::string form);

//获取accessToken
std::string GetAccessToken(std::string yibanUsername, std::string yibanPassword);

//获取授权URL
std::string GetAuthURL(std::string accessToken);

//获取x-auth-token
std::string GetXAuthToken(std::string authURL);

//完整获取x-auth-token请求
std::string totalXAuthToken(std::string yibanUsername, std::string yibanPassword);

std::string GetValidationCode(std::string signInCode, std::string xAuthToken);

std::string SendValidationCode(std::string imageBase64);

std::string totalValidationCode(std::string signInCode, std::string xAuthToken);

std::string SignInRequest(std::string validationCode, std::string xAuthToken);

void exceptionTest();
