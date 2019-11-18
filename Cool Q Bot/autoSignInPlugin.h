#include "httplib.h"
#include <ctime>

std::mutex multiMutex;
namespace stea::httpRequest
{
    /// 函数声明
    std::string totalValidationCode(std::string signInCode, std::string xAuthToken);
    std::string GetValidationCode(std::string signInCode, std::string xAuthToken);
    std::string SendValidationCode(std::string imageBase64);
    std::string SignInRequest(std::string validationCode, std::string xAuthToken);

    std::string totalXAuthToken(std::string yibanUsername, std::string yibanPassword);
    std::string GetAccessToken(std::string yibanUsername, std::string yibanPassword);
    std::string GetAuthURL(std::string accessToken);
    std::string GetXAuthToken(std::string authPath);

    /// 函数定义
    std::string totalValidationCode(std::string signInCode, std::string xAuthToken)
    {
        //long long startTime = GetTickCount();
        //long long endTime;
        std::string imageBase64, validationCode;
        try
        {
            imageBase64 = GetValidationCode(signInCode, xAuthToken);
            //endTime = GetTickCount();
            //cq::logging::debug(u8"HTTP", u8"ON_BASE64: " + std::to_string(endTime - startTime) + "ms");
        }
        catch(std::string errorString)
        {
            throw errorString;
        }
        catch(const char *errorChar)
        {
            throw errorChar;
        }
        catch(unsigned int statusCode)
        {
            throw std::to_string(statusCode);
        }
        validationCode = SendValidationCode(imageBase64);
        //endTime = GetTickCount();
        //cq::logging::debug(u8"HTTP", u8"ON_VALIDATION_CODE: " + std::to_string(endTime - startTime) + "ms");
        return SignInRequest(validationCode, xAuthToken);
    }
    std::string GetValidationCode(std::string signInCode, std::string xAuthToken)
    {
        std::string path = "/api/checkIn/code-check-in?code=" + signInCode + "&w=70&h=26";
        httplib::Client cli("skl.hdu.edu.cn");
        httplib::Headers headers = {
            { "Pragma", "no-cache" },
            { "Cache-Control", "no-cache" },
            { "Accept", "application/json, text/plain, */*" },
            { "X-Auth-Token", xAuthToken },
            { "Accept-Language", "zh-CN,en-US;q=0.9" },
            { "X-Requested-With", "com.yiban.app" }
        };
        auto res = cli.Get(path.c_str(), headers);
        if (res && res->status == 200)
        {
            std::string responseBody = res->body, imageBase64 = "";
            //cq::logging::debug(u8"HTTP", u8"imageBlob: " + responseBody);
            const unsigned char *imageBlob = (unsigned char *)(responseBody.c_str());
            imageBase64 = cq::utils::base64::encode(imageBlob,responseBody.size());
            //cq::logging::debug(u8"HTTP", u8"imageBase64: " + imageBase64);
            return imageBase64;
        }
        else if(res && res->status == 403)
        {
            std::string errorString = "EXPIRED_TOKEN";
            const char *errorChar = errorString.c_str();
            throw errorChar;
        }
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
    }
    std::string SendValidationCode(std::string imageBase64)
    {
        std::string old_value = "/", new_value = "-";
        int pos = 0;
        while((pos = imageBase64.find(old_value)) != std::string::npos)
        {
            imageBase64 = imageBase64.replace(pos, old_value.length() , new_value);
        }
        std::string path = "/api/cap/" + imageBase64;
        httplib::Client cli("127.0.0.1");
        auto res = cli.Get(path.c_str());
        if (res)
        {
            std::string responseBody = res->body;
            //cq::logging::debug(u8"validationCode", responseBody);
            return responseBody;
        }
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
    }
    std::string SignInRequest(std::string validationCode, std::string xAuthToken)
    {
        std::string path = "/api/checkIn/valid-code?code=" + validationCode;
        httplib::Client cli("skl.hdu.edu.cn");
        httplib::Headers headers = {
            { "Pragma", "no-cache" },
            { "Cache-Control", "no-cache" },
            { "Accept", "application/json, text/plain, */*" },
            { "X-Auth-Token", xAuthToken },
            { "User-Agent", "Mozilla/5.0 (Linux; Android 9; ONEPLUS A6003 Build/PQ3B.190801.002; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/77.0.3865.92 Mobile Safari/537.36 yiban_android" },
            { "Referer", "https://skl.hduhelp.com/?type=2" },
            { "Accept-Encoding", "zip, deflate" },
            { "Accept-Language", "zh-CN,en-US;q=0.9" },
            { "X-Requested-With", "com.yiban.app" }
        };
        auto res = cli.Get(path.c_str(), headers);
        if (res)
        {
            std::string responseCode = std::to_string(res->status);
            //cq::logging::debug(u8"HTTP", u8"responseCode: " + responseCode);
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
            else
            {
                return "UNKNOWN_ERROR";
            }
        }
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
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
        std::string path = "/api/v2/passport/login?account=" + yibanUsername + "&passwd=" + yibanPassword + "&ct=2&app=1&v=4.6.2&apn=3gnet&identify=867520044297975&sig=316ab653300d5108&token=&device=OnePlus%3AONEPLUS+A6003&sversion=28";
        httplib::Client cli("mobile.yiban.cn");
        httplib::Headers headers = {
            { "Content-type", "text/json" },
            { "Authorization", "Bearer" },
            { "loginToken", "" },
            { "AppVersion", "4.6.2" }
        };
        auto res = cli.Get(path.c_str(), headers);
        if (res)
        {
            std::string responseBody = res->body;
            //cq::logging::debug(u8"HTTP", u8"GetAccessToken: " + responseBody);
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
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
        
    }
    std::string GetAuthURL(std::string accessToken)
    {
        int pos;
        std::string path = "/iapp/index?act=iapp319528&v=" + accessToken;
        httplib::Client cli("f.yiban.cn");
        httplib::Headers headers = {
            { "Content-type", "text/json" },
            { "Upgrade-Insecure-Requests", "1" },
            { "Referer", "https://oauth.yiban.cn/code/html?client_id=dc76b1d4eba657b0&redirect_uri=http://f.yiban.cn/iapp319528" },
            { "Cache-Control", "no-cache" },
            { "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8" },
            { "Upgrade-Insecure-Requests", "1" },
            { "Cookie", "client=android; _YB_OPEN_V2_0=r2130lIb0h1env80; yibanM_user_token=" + accessToken },
            { "X-Requested-With", "com.yiban.app" }
        };
        auto res = cli.Get(path.c_str(), headers);
        if (res)
        {
            std::string Location = res->get_header_value("Location");
            //cq::logging::debug(u8"HTTP", u8"Location: " + Location);
            if ((pos = Location.find("verify_request=")) != std::string::npos)
            {
                std::string authPath = Location.substr(pos);
                //pos = authPath.find("\r\n");
                //authPath = authPath.erase(pos);
                //cq::logging::debug(u8"HTTP", u8"authPath: " + authPath);
                return authPath;
            }
            else
            {
                std::string errorString = "AUTH_ERROR";
                throw errorString;
            }
        }
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
        
    }
    std::string GetXAuthToken(std::string authPath)
    {
        std::string path = "/api/login/yiban?" + authPath;
        httplib::Client cli("skl.hdu.edu.cn");
        auto res = cli.Get(path.c_str());
        if (res)
        {
            std::string xAuthToken = res->get_header_value("X-Auth-Token");
            //cq::logging::debug(u8"HTTP", u8"xAuthToken: " + xAuthToken);
            return xAuthToken;
        }
        else
        {
            std::string errorString = "UNKNOWN_ERROR";
            throw errorString;
        }
    }
}
namespace stea::fileio
{
    /// 定义 用户数据 类
    class userDatas
	{
	public:
		unsigned int readTotalUserCounts();
		static void addTotalGroupCounts();
        static void minusTotalGroupCounts();
		static unsigned int readTotalGroupCounts();
		
		std::string readGroupNumber();
		std::string readUserQQs(int iterator);
		std::string readUserAccounts(int iterator);
		std::string readUserPasswords(int iterator);
		std::string readUserXAuthTokens(int iterator);

		void setGroupNumber(std::string groupNumberInput);
        void setUserPasswords(unsigned int iterator, std::string userPassword);
        void setUserXAuthTokens(unsigned int iterator, std::string userXAuthToken);
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
    void userDatas::minusTotalGroupCounts()
	{
		totalGroupCounts--;
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
    void userDatas::setUserXAuthTokens(unsigned int iterator, std::string userXAuthToken)
	{
		userXAuthTokens[iterator] = userXAuthToken;
	}
    void userDatas::setUserPasswords(unsigned int iterator, std::string userPassword)
	{
		userPasswords[iterator] = userPassword;
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
        //cq::logging::debug(u8"LOGOUT", u8"尝试删除用户，用户ID：" + iterator);
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
        for (unsigned int i = 0; i < stea::fileio::userDatas::readTotalGroupCounts(); i++)
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

void multiSignIn(std::vector<stea::fileio::userDatas>& groupDatas, unsigned int groupIterator, unsigned int userIterator, std::string signInCode, std::string& stateMsg);

void multiSignIn(std::vector<stea::fileio::userDatas>& groupDatas, unsigned int groupIterator, unsigned int userIterator, std::string signInCode, std::string& stateMsg)
{
    //std::lock_guard<std::mutex> mutex(multiMutex);
    std::string tempState, userQQ = groupDatas[groupIterator].readUserQQs(userIterator);
    try
    {
        tempState = stea::httpRequest::totalValidationCode(signInCode, groupDatas[groupIterator].readUserXAuthTokens(userIterator));
        stateMsg += "\n    用户QQ： [CQ:at,qq=" + userQQ + "]\n        返回：" + tempState;
        //cq::logging::debug(u8"GROUP", tempState);
    }
    catch(std::string errorString)
    {
        tempState = errorString;
        stateMsg += "\n    用户QQ： [CQ:at,qq=" + userQQ + "]\n        错误提示：" + errorString;
        //cq::logging::debug(u8"GROUP", tempState);
    }
    catch(const char *errorChar)
    {
        tempState = "";
        try
        {
            tempState += "\n    用户QQ： [CQ:at,qq=" + userQQ + "]\n        错误提示：EXPIRED_TOKEN";
            //cq::logging::debug(u8"GROUP", u8"userAccount: " + groupDatas[groupIterator].readUserAccounts(userIterator));
            //cq::logging::debug(u8"GROUP", u8"userPassword: " + groupDatas[groupIterator].readUserPasswords(userIterator));
            groupDatas[groupIterator].setUserXAuthTokens(userIterator, stea::httpRequest::totalXAuthToken(groupDatas[groupIterator].readUserAccounts(userIterator), groupDatas[groupIterator].readUserPasswords(userIterator)));
            tempState += "\n        用户Token已更新：" + groupDatas[groupIterator].readUserXAuthTokens(userIterator);
            //cq::logging::debug(u8"GROUP", u8"New Token" + xAuthToken);
            tempState += "\n        重新签到：";
            tempState += "\n            返回：" + stea::httpRequest::totalValidationCode(signInCode, groupDatas[groupIterator].readUserXAuthTokens(userIterator)); // Message 类可以进行加法运算

            stea::fileio::saveUserDataStream(groupDatas);
        }
        catch(std::string errorString)
        {
            tempState += "\n            错误提示：" + errorString;
        }
        catch(const char *errorChar)
        {
            tempState += "\n            错误提示：UNKNOWN_ERROR";
        }
        catch(unsigned int statusCode)
        {
            tempState += "\n            错误码：" + std::to_string(statusCode);
        }
        stateMsg += tempState;
    }
    catch(unsigned int statusCode)
    {
        tempState = std::to_string(statusCode);
        stateMsg += "\n    用户QQ： [CQ:at,qq=" + userQQ + "]\n        错误码：" + std::to_string(statusCode);
    }
    //promiseStateMsg.set_value(stateMsg);
    //cq::api::send_group_msg(628669530, stateMsg);
}