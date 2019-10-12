#include "networkRequest.h"
#include <fstream>

namespace cq::httpRequest
{
    /// 函数声明
    void multiSignIn(std::string userQQ, std::string signInCode, std::string xAuthToken);
    std::string totalValidationCode(std::string signInCode, std::string xAuthToken);
    std::string GetValidationCode(std::string signInCode, std::string xAuthToken);
    std::string SendValidationCode(std::string imageBase64);
    std::string SignInRequest(std::string validationCode, std::string xAuthToken);

    std::string totalXAuthToken(std::string yibanUsername, std::string yibanPassword);
    std::string GetAccessToken(std::string yibanUsername, std::string yibanPassword);
    std::string GetAuthURL(std::string accessToken);
    std::string GetXAuthToken(std::string authPath);

    /// 函数定义
    void multiSignIn(std::string userQQ, std::string signInCode, std::string xAuthToken)
    {
        cq::logging::debug(u8"Multy", u8"QQ：" + userQQ + u8"Token: " + xAuthToken);
        std::string tempState;
        try
        {
            tempState = totalValidationCode(signInCode, xAuthToken);
        }
        catch(std::string errorString)
        {
            tempState = errorString;
        }
        catch(unsigned int statusCode)
        {
            tempState = std::to_string(statusCode);
        }
        cq::logging::debug(u8"Multy", u8"返回：" + tempState);
    }

    std::string totalValidationCode(std::string signInCode, std::string xAuthToken)
    {
        std::string imageBase64, validationCode;
        try
        {
            imageBase64 = GetValidationCode(signInCode, xAuthToken);
        }
        catch(std::string errorString)
        {
            return errorString;
        }
        catch(unsigned int statusCode)
        {
            return "INVALID_SIGNIN_ATTEMPT: STATUS_CODE = " + std::to_string(statusCode);
        }
        validationCode = SendValidationCode(imageBase64);
        return SignInRequest(validationCode, xAuthToken);
    }
    std::string GetValidationCode(std::string signInCode, std::string xAuthToken)
    {
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
        try
        {
            responseBody = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeaders, "BODY", 5, 12);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        const unsigned char *imageBlob = (unsigned char *)(responseBody.c_str());
        imageBase64 = cq::utils::base64::encode(imageBlob,responseBody.size());
        //cq::logging::debug(u8"imageBase64", imageBase64);
        return imageBase64;
    }
    std::string SendValidationCode(std::string imageBase64)
    {
        std::string old_value = "/", new_value = "-";
        int pos = 0;
        while((pos = imageBase64.find(old_value)) != std::string::npos)
        {
            imageBase64 = imageBase64.replace(pos, old_value.length() , new_value);
        }
        std::string responseBody = "";
        std::string requestHost = "localhost";
        std::string requestPath = "/api/cap/" + imageBase64;
        std::string requestHeader = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3\r\n"
            "Accept-Encoding: gzip, deflate, br\r\n"
            "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n"
            "Connection: close\r\n";
        //cq::logging::debug(u8"requestBody", requestPath);
        try
        {
            responseBody = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeader, "BODY", 0, 0);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
		catch(unsigned int statusCode)
        {
			throw statusCode;
		}
        cq::logging::debug(u8"validationCode", responseBody);
        return responseBody;
    }
    std::string SignInRequest(std::string validationCode, std::string xAuthToken)
    {
        std::string responseCode = "";
        std::string requestHost = "skl.hdu.edu.cn";
        std::string requestPath = "/api/checkIn/valid-code?code=" + validationCode;
        std::string requestHeaders = "Connection: close\r\n"
            "Pragma: no-cache\r\n"
            "Cache-Control: no-cache\r\n"
            "Accept: application/json, text/plain, */*\r\n"
            "X-Auth-Token: " + xAuthToken + "\r\n"
            "User-Agent: Mozilla/5.0 (Linux; Android 9; ONEPLUS A6003 Build/PQ3B.190801.002; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/77.0.3865.92 Mobile Safari/537.36 yiban_android\r\n"
            "Sec-Fetch-Mode: cors"
            "X-Requested-With: com.yiban.app"
            "Sec-Fetch-Site: cross-site"
            "Referer: https://skl.hduhelp.com/?type=2\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Accept-Language: zh-CN,zh;en-US;q=0.9\r\n";
        try
        {
            responseCode = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeaders, "CODE", 5, 12);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        if(responseCode == "200")
        {
            return "SUCCESS";
        }
        else if(responseCode == "401")
        {
            return "INVALID_SIGNIN_CODE";
        }
        else if(responseCode == "400")
        {
            return "INVALID_VALIDATION_CODE";
        }
        else if(responseCode == "403")
        {
            return "INVALID_TOKEN";
        }
        //cq::logging::debug(u8"ASIO", u8"responseCode=" + responseCode);
        return "UNKNOWN_ERROR";
    }
    std::string totalXAuthToken(std::string yibanUsername, std::string yibanPassword)
    {
        std::string accessToken, authPath, xAuthToken;
        try
        {
            accessToken = GetAccessToken(yibanUsername, yibanPassword);
            authPath = GetAuthURL(accessToken);
            xAuthToken = GetXAuthToken(authPath);
            //cq::logging::debug(u8"ASIO", u8"调用totalXAuthToken,token=" + xAuthToken);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        return xAuthToken;
    }
    std::string GetAccessToken(std::string yibanUsername, std::string yibanPassword)
    {
        int pos;
        std::string responseBody = "";
        std::string requestHost = "mobile.yiban.cn";
        std::string requestPath = "/api/v2/passport/login?account=" + yibanUsername + "&passwd=" + yibanPassword + "&ct=2&app=1&v=4.6.2&apn=3gnet&identify=867520044297975&sig=316ab653300d5108&token=&device=OnePlus%3AONEPLUS+A6003&sversion=28";
        std::string requestHeaders = "Content-type:text/json\r\n"
                                    "Authorization: Bearer\r\n"
                                    "loginToken:\r\n"
                                    "AppVersion: 4.6.2\r\n"
                                    "Connection: close\r\n";
        try
        {
            responseBody = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeaders, "BODY", 5, 12);
            //cq::logging::debug(u8"ASIO", u8"调用GetAccessToken,responseBody = " + responseBody);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        if ((pos = responseBody.find("access_token=")) != std::string::npos)
        {
            std::string accessToken = responseBody.substr(pos + 13);
            accessToken = accessToken.erase(32);
            //cq::logging::debug(u8"ASIO", u8"accessToken = " + accessToken);
            return accessToken;
        }
        else if ((pos = responseBody.find("response")) != std::string::npos)
        {
            std::string responseCode = responseBody.substr(pos + 10);
            responseCode = responseCode.erase(3);
            if(responseCode == "405")
            {
                responseCode = "MISSING_PARAMETER";
                throw responseCode;
            }
            else if(responseCode == "415")
            {
                responseCode = "WRONG_PASSWORD";
                throw responseCode;
            }
            else
            {
                responseCode = "UNKNOWN_LOGIN_EXCEPTION";
                throw responseCode;
            }
        }
        else
        {
            const std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
    }
    std::string GetAuthURL(std::string accessToken)
    {
        //cq::logging::debug(u8"ASIO", u8"调用GetAuthURL");
        int pos;
        std::string responseHeaders = "";
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
        try
        {
            responseHeaders = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeaders, "HEADER", 5, 12);
            //cq::logging::debug(u8"ASIO", u8"调用GetAuthURL,responseHeaders = " + responseHeaders);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        if ((pos = responseHeaders.find("verify_request=")) != std::string::npos)
        {
            std::string authPath = responseHeaders.substr(pos);
            pos = authPath.find("\r\n");
            authPath = authPath.erase(pos);
            //cq::logging::debug(u8"ASIO", u8"authPath = " + authPath);
            return authPath;
        }
        else
        {
            std::string errorString = "AUTH_ERROR";
            throw errorString;
        }
    }
    std::string GetXAuthToken(std::string authPath)
    {
        int pos;
        std::string responseBody = "";
        std::string requestHost = "skl.hdu.edu.cn";
        std::string requestPath = "/api/login/yiban?" + authPath;
        std::string requestHeaders = "Connection: close\r\n";
        try
        {
            responseBody = stea::asio::httpRequests::getRequest(requestPath, requestHost, requestHeaders, "HEADER", 5, 12);
            //cq::logging::debug(u8"ASIO", u8"调用GetXAuthToken,responseBody = " + responseBody);
        }
        catch(std::string errorString)
        {
			throw errorString;
		}
        catch(unsigned int statusCode)
        {
            throw statusCode;
        }
        if ((pos = responseBody.find("X-Auth-Token:")) != std::string::npos)
        {
            std::string xAuthToken = responseBody.substr(pos + 14);
            pos = xAuthToken.find("\r\n");
            xAuthToken = xAuthToken.erase(pos);
            //cq::logging::debug(u8"ASIO", u8"xAuthToken = " + xAuthToken);
            return xAuthToken;
        }
        else
        {
            throw "TOKEN_ERROR";
        }
    }
}
namespace cq::fileio
{
    /// 定义 用户数据 类
    class userDatas
	{
	public:
		unsigned int readTotalUserCounts();
		static void addTotalGroupCounts();
		static unsigned int readTotalGroupCounts();
		
		std::string readGroupNumber();
		std::string readUserQQs(int iterator);
		std::string readUserAccounts(int iterator);
		std::string readUserPasswords(int iterator);
		std::string readUserXAuthTokens(int iterator);

		void setGroupNumber(std::string groupNumberInput);
		void addBackUserQQs(std::string userQQ);
		void addBackUserAccounts(std::string userAccount);
		void addBackUserPasswords(std::string userPassword);
		void addBackUserXAuthTokens(std::string userXAuthToken);

        void deleteUserAccount(int iterator);
	private:
		std::string groupNumber;
		std::vector<std::string> userQQs, userAccounts, userPasswords, userXAuthTokens;
		static unsigned int totalGroupCounts;

	};

#pragma region CLASS_FUNCTION_DEFINES
    /// 全局函数
	unsigned int userDatas::readTotalUserCounts()
	{
		return userQQs.size();
	}
    void userDatas::addTotalGroupCounts()
	{
		totalGroupCounts++;
	}
	unsigned int userDatas::readTotalGroupCounts()
	{
		return totalGroupCounts;
	}
    /// 读取操作
	std::string userDatas::readGroupNumber()
	{
		return groupNumber;
	}
	std::string userDatas::readUserQQs(int iterator)
	{
		return userQQs[iterator];
	}
	std::string userDatas::readUserAccounts(int iterator)
	{
		return userAccounts[iterator];
	}
	std::string userDatas::readUserPasswords(int iterator)
	{
		return userPasswords[iterator];
	}
	std::string userDatas::readUserXAuthTokens(int iterator)
	{
		return userXAuthTokens[iterator];
	}
    /// 写入操作
	void userDatas::setGroupNumber(std::string groupNumberInput)
	{
		groupNumber = groupNumberInput;
	}
	void userDatas::addBackUserQQs(std::string userQQ)
	{
		userQQs.push_back(userQQ);
	}
	void userDatas::addBackUserAccounts(std::string userAccount)
	{
		userAccounts.push_back(userAccount);
	}
	void userDatas::addBackUserPasswords(std::string userPassword)
	{
		userPasswords.push_back(userPassword);
	}
	void userDatas::addBackUserXAuthTokens(std::string userXAuthToken)
	{
		userXAuthTokens.push_back(userXAuthToken);
	}

    /// 删除操作
    void userDatas::deleteUserAccount(int iterator)
    {
        cq::logging::debug(u8"LOGOUT", u8"尝试删除用户，用户ID：" + iterator);
        userQQs.erase(userQQs.begin()+iterator);
        userAccounts.erase(userAccounts.begin()+iterator);
        userPasswords.erase(userPasswords.begin()+iterator);
        userXAuthTokens.erase(userXAuthTokens.begin()+iterator);
    }
#pragma endregion CLASS_FUNCTION_DEFINES

	///函数声明
	void loadUserDataStream(std::vector<userDatas>& userDatasVector);
    void saveUserDataStream(std::vector<userDatas> userDatasVector);

	///函数定义
	void loadUserDataStream(std::vector<userDatas>& userDatasVector)
	{
		std::ifstream userDataStream("./data/app/cn.hdustea.autosigninplugin/userDatas.ini", std::ios::in);
		std::string tempLine;
		while (getline(userDataStream, tempLine))
		{
            userDatas tempUserDatas;
			//std::cout << tempLine << std::endl;
			std::stringstream tempLineStream(tempLine);
			std::string tempBlockString;
			unsigned int tempDataType = 0;
			// 按照逗号分隔
			while (getline(tempLineStream, tempBlockString, ';'))
			{
				std::stringstream tempBlockStream(tempBlockString);
				std::string tempString;
				unsigned int tempGroupCount = userDatas::readTotalGroupCounts();
				switch (tempDataType)
				{
				case 0:
					//std::cout << "groupNumber:" << std::endl;
					while (getline(tempBlockStream, tempString, ','))
					{
						tempUserDatas.setGroupNumber(tempString);
					}
					break;
				case 1:
					//std::cout << "userQQs:" << std::endl;
					while (getline(tempBlockStream, tempString, ','))
					{
						tempUserDatas.addBackUserQQs(tempString);
					}
					break;
				case 2:
					//std::cout << "userAccounts:" << std::endl;
					while (getline(tempBlockStream, tempString, ','))
					{
						tempUserDatas.addBackUserAccounts(tempString);
					}
					break;
				case 3:
					//std::cout << "userPasswords:" << std::endl;
					while (getline(tempBlockStream, tempString, ','))
					{
						tempUserDatas.addBackUserPasswords(tempString);
					}
					break;
				case 4:
					//std::cout << "userXAuthTokens:" << std::endl;
					while (getline(tempBlockStream, tempString, ','))
					{
						tempUserDatas.addBackUserXAuthTokens(tempString);
					}
					break;
				default:
					break;
				}
				tempDataType++;
			}
			userDatas::addTotalGroupCounts();
            userDatasVector.push_back(tempUserDatas);
		}
		userDataStream.close();
	}

    void saveUserDataStream(std::vector<userDatas> userDatasVector)
	{
		std::ofstream userDataStream("./data/app/cn.hdustea.autosigninplugin/userDatas.ini", std::ios::out);

		std::string userDataMessage = "";
        for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
        {
            userDataMessage += userDatasVector[i].readGroupNumber() + ";";
            for (unsigned int j = 0; j < userDatasVector[i].readTotalUserCounts(); j++)
            {
                if(j == userDatasVector[i].readTotalUserCounts()-1)
                {
                    userDataMessage += userDatasVector[i].readUserQQs(j);
                }
                else
                {
                    userDataMessage += userDatasVector[i].readUserQQs(j) + ",";
                }
            }
            userDataMessage += ";";
            for (unsigned int j = 0; j < userDatasVector[i].readTotalUserCounts(); j++)
            {
                if(j == userDatasVector[i].readTotalUserCounts()-1)
                {
                    userDataMessage += userDatasVector[i].readUserAccounts(j);
                }
                else
                {
                    userDataMessage += userDatasVector[i].readUserAccounts(j) + ",";
                }
            }
            userDataMessage += ";";
            for (unsigned int j = 0; j < userDatasVector[i].readTotalUserCounts(); j++)
            {
                if(j == userDatasVector[i].readTotalUserCounts()-1)
                {
                    userDataMessage += userDatasVector[i].readUserPasswords(j);
                }
                else
                {
                    userDataMessage += userDatasVector[i].readUserPasswords(j) + ",";
                }
            }
            userDataMessage += ";";
            for (unsigned int j = 0; j < userDatasVector[i].readTotalUserCounts(); j++)
            {
                if(j == userDatasVector[i].readTotalUserCounts()-1)
                {
                    userDataMessage += userDatasVector[i].readUserXAuthTokens(j);
                }
                else
                {
                    userDataMessage += userDatasVector[i].readUserXAuthTokens(j) + ",";
                }
            }
            userDataMessage += "\n";
        }
        userDataStream << userDataMessage;
		userDataStream.close();
	}
}
/*
namespace cq::python
{
    bool pythonInit();

    bool pythonInit()
    {
        Py_Initialize();
        return Py_IsInitialized();
    }
}
*/