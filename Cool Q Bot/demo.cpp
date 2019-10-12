#include "cqsdk/cqsdk.h"
//#include "pythonsdk/Python.h"
#include "autoSignInPlugin.h"

#define SHELL

#ifndef SHELL
	#define NATURAL
#endif

// namespace cq::app 包含插件本身的生命周期事件和管理
// namespace cq::event 用于注册 QQ 相关的事件处理函数
// namespace cq::api 用于调用酷 Q 提供的接口
// namespace cq::logging 用于日志
// namespace cq::message 提供封装了的 Message 等类

const int administrator = 1135989508, mainGroup = 628669530;

unsigned int cq::fileio::userDatas::totalGroupCounts = 0;
std::vector<cq::fileio::userDatas> groupDatas;

long long startRuntime = 0;

// 插件入口，在静态成员初始化之后，app::on_initialize 事件发生之前被执行，用于配置 SDK 和注册事件回调
CQ_MAIN
{
    cq::app::on_enable = []
	{
        // cq::logging、cq::api、cq::dir 等命名空间下的函数只能在事件回调函数内部调用，而不能直接在 CQ_MAIN 中调用
		startRuntime= GetTickCount();
		/*
		if(cq::python::pythonInit())
		{
			cq::logging::debug(u8"PYTHON", u8"Python initiated!");
		}
		else
		{
			cq::logging::debug(u8"PYTHON", u8"Python initiation failed!");
		}
		*/
		cq::logging::debug(u8"ENABLE", u8"Auto_Sign_In_Plugin is enabled!");

		cq::fileio::loadUserDataStream(groupDatas);

		try
		{
			/*
			std::string userDataMessage = "";
			for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
			{
				userDataMessage += u8"群号: " + groupDatas[i].readGroupNumber() + "\n";
				userDataMessage += u8"用户人数: " + std::to_string(groupDatas[i].readTotalUserCounts()) + "\n";
				for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
				{
					userDataMessage += u8"    用户QQ: " + groupDatas[i].readUserQQs(j) + "\n";
					userDataMessage += u8"    用户账号: " + groupDatas[i].readUserAccounts(j) + "\n";
					userDataMessage += u8"    用户密码: " + groupDatas[i].readUserPasswords(j) + "\n";
					userDataMessage += u8"    用户Token: " + groupDatas[i].readUserXAuthTokens(j) + "\n\n";
				}
			}
			long long endRuntime = GetTickCount();
			cq::api::send_private_msg(administrator, userDataMessage + u8"执行耗时：" + std::to_string(endRuntime - startRuntime) + "ms");
			*/
			long long endRuntime = GetTickCount();
			cq::api::send_group_msg(mainGroup, u8"丢人插件正在运行\n执行耗时：" + std::to_string(endRuntime - startRuntime) + "ms");
		}
		catch (const cq::exception::ApiError &err)
		{
            // API 调用失败
            cq::logging::debug(u8"API", u8"调用失败，错误码：" + std::to_string(err.code));
        }
    };

    cq::event::on_private_msg = [](const cq::PrivateMessageEvent &e)
	{
		startRuntime = GetTickCount();
		using namespace std;
		//int position;
		//正则匹配

		#ifdef SHELL
			regex regexHelp("#help");
			regex regexPing("#ping");
			regex regexRebootAttempt("#reboot");

			regex regexListAccounts("#list");
			regex regexSaveAttempt("#save");

			regex regexMultiSignIn("#all (\\d{4})");
		#endif

		#ifdef NATURAL
			regex regexHelp("插件帮助");
			regex regexPing("ping");
			regex regexRebootAttempt("重启机器人");

			regex regexListAccounts("用户列表");
			regex regexSaveAttempt("保存数据");

			regex regexMultiSignIn("a(\\d{4})");
		#endif

		smatch result;

        //cq::logging::debug(u8"消息", u8"收到私聊消息：" + e.message + u8"，发送者：" + std::to_string(e.user_id));

		if(regex_match(e.raw_message,result,regexHelp))
		{
			cq::Message msg = u8"丢人插件帮助：\n";
			#ifdef SHELL
				msg += u8"    大括号{}用来指代需要自己输入的部分，使用时请不要带大括号\n";
				msg += u8"#help\n";
				msg += u8"    显示此帮助菜单\n";
				msg += u8"#ping\n";
				msg += u8"    简单的Ping，返回Pong！\n";
				msg += u8"#list\n";
				msg += u8"    显示所有用户列表\n";
				msg += u8"#save\n";
				msg += u8"    即时保存用户列表\n";
				msg += u8"#all {code}\n";
				msg += u8"    为所有人签到（尚未实装）\n";
			#endif

			#ifdef NATURAL
				msg += u8"    大括号{}用来指代需要自己输入的部分，使用时请不要带大括号\n";
				msg += u8"插件帮助\n";
				msg += u8"    显示此帮助菜单\n";
				msg += u8"ping\n";
				msg += u8"    简单的Ping，返回Pong！\n";
				msg += u8"新建签到 {account} {password}\n";
				msg += u8"    登录并获取Token\n";
				msg += u8"用户列表\n";
				msg += u8"    显示本群用户列表\n";
				msg += u8"保存数据\n";
				msg += u8"    即时保存用户列表\n";
				msg += u8"注销签到\n";
				msg += u8"    注销并从列表删除\n";
				msg += u8"s{code}\n";
				msg += u8"    个人签到\n";
				msg += u8"a{code}\n";
				msg += u8"    为本群所有人签到（尚未实装）\n";
			#endif

			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
		}
		/*
        try {
            cq::api::send_private_msg(e.user_id, e.message); // echo 回去

            cq::api::send_msg(e.target, e.message); // 使用 e.target 指定发送目标

            // MessageSegment 类提供一些静态成员函数以快速构造消息段
            cq::Message msg = cq::MessageSegment::contact(cq::MessageSegment::ContactType::GROUP, 628669530);
            msg.send(e.target); // 使用 Message 类的 send 成员函数
        } catch (const cq::exception::ApiError &err)
		{
            // API 调用失败
            cq::logging::debug(u8"API", u8"调用失败，错误码：" + std::to_string(err.code));
        }
		*/

        e.block(); // 阻止事件继续传递给其它插件
    };

    cq::event::on_group_msg = [](const auto &e /* 使用 C++ 的 auto 关键字 */)
	{
		startRuntime = GetTickCount();
		using namespace std;
		
		cq::logging::debug(u8"GROUP_RAW", e.raw_message);
		cq::logging::debug(u8"GROUP", e.message);
		//正则匹配

		#ifdef SHELL
			regex regexHelp("#[h?？]");
			regex regexRecall("#rc (\\d{1,2})");
			regex regexPing("#cyka");
			regex regexRebootAttempt("#reboot");

			regex regexLoginAttempt("#login (\\d{11}) (\\S{6,19})");
			regex regexListAccounts("#ls");
			regex regexSaveAttempt("#save");

			regex regexLogoutOther("#logout \\[CQ:at,qq=(\\d{5,20})\\]\\s*");
			regex regexLogoutSelf("#logout");
			regex regexSelfSignIn("#s (\\d{4})");
			regex regexOtherSignIn("#o (\\d{4}) \\[CQ:at,qq=(\\d{5,20})\\]\\s*");
			regex regexMultiSignIn("#a (\\d{4})");
		#endif

		#ifdef NATURAL
			regex regexHelp("插件帮助");
			regex regexRecall("撤回 (\\d{1,2})");
			regex regexPing("ping");
			regex regexRebootAttempt("重启机器人");

			regex regexLoginAttempt("新建签到 (\\d{11}) (\\S{6,19})");
			regex regexListAccounts("用户列表");
			regex regexSaveAttempt("保存数据");

			regex regexLogoutOther("注销签到\\[CQ:at,qq=(\\d{6,13})\\]\\s*");
			regex regexLogoutSelf("注销签到");
			regex regexSelfSignIn("s(\\d{4})");
			regex regexOtherSignIn("o(\\d{4})\\[CQ:at,qq=(\\d{6,13})\\]\\s*");
			regex regexMultiSignIn("a(\\d{4})");
		#endif

		smatch result;

		if(regex_match(e.raw_message,result,regexHelp))
		{
			cq::Message msg = u8"丢人插件帮助：\n";
			#ifdef SHELL
				msg += u8"    大括号{}用来指代需要自己输入的部分，使用时请不要带大括号\n";
				msg += u8"#h\n";
				msg += u8"    显示此帮助菜单\n";
				msg += u8"#cyka\n";
				msg += u8"    简单的Ping，返回Blyat！\n";
				msg += u8"#login {account} {password}\n";
				msg += u8"    登录并获取Token\n";
				msg += u8"#ls\n";
				msg += u8"    显示本群用户列表\n";
				msg += u8"#save\n";
				msg += u8"    即时保存用户列表\n";
				msg += u8"#logout\n";
				msg += u8"    注销并从列表删除\n";
				msg += u8"#s {code}\n";
				msg += u8"    个人签到\n";
				msg += u8"#o {code} {@someone}\n";
				msg += u8"    为他人签到\n";
				msg += u8"#a {code}\n";
				msg += u8"    为本群所有人签到（尚未实装）\n";
			#endif

			#ifdef NATURAL
				msg += u8"    大括号{}用来指代需要自己输入的部分，使用时请不要带大括号\n";
				msg += u8"插件帮助\n";
				msg += u8"    显示此帮助菜单\n";
				msg += u8"cyka\n";
				msg += u8"    简单的Ping，返回Blyat！\n";
				msg += u8"新建签到 {account} {password}\n";
				msg += u8"    登录并获取Token\n";
				msg += u8"用户列表\n";
				msg += u8"    显示本群用户列表\n";
				msg += u8"保存数据\n";
				msg += u8"    即时保存用户列表\n";
				msg += u8"注销签到\n";
				msg += u8"    注销并从列表删除\n";
				msg += u8"s{code}\n";
				msg += u8"    个人签到\n";
				msg += u8"a{code}\n";
				msg += u8"    为本群所有人签到（尚未实装）\n";
			#endif

			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
		}
		if(regex_match(e.raw_message,result,regexRecall))
		{
			string message_id = result[1];
			cq::Message msg = "";
			try
			{
				cq::api::delete_msg(e.message_id - atoi(message_id.c_str()));
				msg += u8"撤回成功！";
			}
			catch (const cq::exception::ApiError &err)
			{
				// API 调用失败
				cq::logging::debug(u8"API", u8"调用失败，错误码：" + std::to_string(err.code));
				msg += u8"撤回失败！";
			}
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
		}
		if(regex_match(e.raw_message,result,regexPing))
		{
			cq::Message msg = u8"Blyat!";
			cq::message::send(e.target, msg);
		}
		if((regex_match(e.raw_message,result,regexRebootAttempt))&&(e.user_id == administrator))
		{
			cq::fileio::saveUserDataStream(groupDatas);
			cq::Message msg = u8"正在执行REBOOT指令，10秒内重启……";
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
			system("taskkill /f /t /im CQP.exe");
		}

		if(regex_match(e.raw_message,result,regexLoginAttempt))
		{
			string groupNumber = to_string(e.group_id), userQQ = to_string(e.user_id), userAccount = result[1], userPassword = result[2], userXAuthToken = "";
			cq::Message msg = u8"用户登录:\n";
			bool exist = false;
			msg += u8"    用户QQ：[CQ:at,qq=" + userQQ + "]\n";
			try
			{
				for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
				{
					if(groupDatas[i].readGroupNumber() == groupNumber)
					{
						for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
						{
							if(groupDatas[i].readUserQQs(j) == userQQ)
							{
								groupDatas[i].deleteUserAccount(j);
								exist = true;
								break;
							}
						}
						if(exist)
						{
							string errorString = "USER_EXISTED";
							cq::api::delete_msg(e.message_id);
							throw errorString;
						}
						else
						{
							userXAuthToken = cq::httpRequest::totalXAuthToken(userAccount, userPassword);
							msg += u8"    X-Auth-Token：" + userXAuthToken + "\n";

							groupDatas[i].addBackUserQQs(userQQ);
							groupDatas[i].addBackUserAccounts(userAccount);
							groupDatas[i].addBackUserPasswords(userPassword);
							groupDatas[i].addBackUserXAuthTokens(userXAuthToken);

							cq::api::delete_msg(e.message_id);
							break;
						}
					}
				}
			}
			catch(string errorString)
			{
				msg += u8"    错误内容：" + errorString + "\n";
			}
			catch(unsigned int statusCode)
			{
				msg += u8"    错误码：" + to_string(statusCode) + "\n";
			}
			catch (const cq::exception::ApiError &err)
			{
				// API 调用失败
				cq::logging::debug(u8"API", u8"调用失败，错误码：" + std::to_string(err.code));
			}
			long long endRuntime = GetTickCount();
			msg += u8"执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			
			cq::message::send(e.target, msg);
		}
		if(regex_match(e.raw_message,result,regexListAccounts))
		{
			try
			{
				string userDataMessage = "";
				for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
				{
					if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
					{
						userDataMessage += u8"群号: " + groupDatas[i].readGroupNumber() + "\n";
						userDataMessage += u8"用户人数: " + to_string(groupDatas[i].readTotalUserCounts()) + "\n";
						for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
						{
							userDataMessage += u8"    用户QQ: " + groupDatas[i].readUserQQs(j) + "\n";
							//userDataMessage += "    用户账号: " + groupDatas[i].readUserAccounts(j) + "\n";
							//userDataMessage += "    用户密码: " + groupDatas[i].readUserPasswords(j) + "\n";
							userDataMessage += u8"    用户Token: " + groupDatas[i].readUserXAuthTokens(j) + "\n\n";
						}
					}
				}
				long long endRuntime = GetTickCount();
				cq::message::send(e.target, userDataMessage + u8"执行耗时：" + to_string(endRuntime - startRuntime) + "ms");
			}
			catch (const cq::exception::ApiError &err)
			{
           		// API 调用失败
            	cq::logging::debug(u8"API", u8"调用失败，错误码：" + to_string(err.code));
        	}
		}
		if(regex_match(e.raw_message,result,regexSaveAttempt))
		{
			cq::fileio::saveUserDataStream(groupDatas);
			cq::Message msg = u8"保存配置成功！";
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
		}

		if((regex_match(e.raw_message,result,regexLogoutOther))&&(e.user_id == administrator))
		{
			string logoutQQ = result[1];
			bool successful = false;
			cq::Message msg = u8"    注销用户\n";
			msg += u8"    用户QQ：[CQ:at,qq=" + logoutQQ + "]\n";
			try
			{
				for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
				{
					if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
					{
						msg += u8"    所在群号: " + groupDatas[i].readGroupNumber() + "\n";
						for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
						{
							if(groupDatas[i].readUserQQs(j) == logoutQQ)
							{
								groupDatas[i].deleteUserAccount(j);
								successful = true;
								break;
							}
						}
					}
				}
				if(successful)
				{
					msg += u8"    注销成功！\n";
				}
				else
				{
					msg += u8"    注销失败！\n";
				}
				long long endRuntime = GetTickCount();
				msg += u8"执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
				cq::message::send(e.target, msg);
			}
			catch (const cq::exception::ApiError &err)
			{
           		// API 调用失败
            	cq::logging::debug(u8"API", u8"调用失败，错误码：" + to_string(err.code));
        	}
		}
		if(regex_match(e.raw_message,result,regexLogoutSelf))
		{
			string logoutQQ = to_string(e.user_id);
			bool successful = false;
			cq::Message msg = u8"注销用户：\n";
			msg += u8"    用户QQ：[CQ:at,qq=" + logoutQQ + "]\n";
			try
			{
				for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
				{
					if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
					{
						msg += u8"    所在群号: " + groupDatas[i].readGroupNumber() + "\n";
						for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
						{
							if(groupDatas[i].readUserQQs(j) == logoutQQ)
							{
								groupDatas[i].deleteUserAccount(j);
								successful = true;
								break;
							}
						}
					}
				}
				if(successful)
				{
					msg += u8"    注销成功！\n";
				}
				else
				{
					msg += u8"    注销失败！\n";
				}
				long long endRuntime = GetTickCount();
				msg += u8"执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
				cq::message::send(e.target, msg);
			}
			catch (const cq::exception::ApiError &err)
			{
           		// API 调用失败
            	cq::logging::debug(u8"API", u8"调用失败，错误码：" + to_string(err.code));
        	}
		}
		if(regex_match(e.raw_message,result,regexSelfSignIn))
		{
			//smatch::iterator x;
			string signInCode = result[1], userQQ = to_string(e.user_id), xAuthToken, statusCode;
			bool successful = false;

			cq::Message msg = u8"个人签到："; // string 到 Message 自动转换
			msg += u8"\n    签到码：" + signInCode; // Message 类可以进行加法运算

			for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
			{
				if(successful)
				{
					break;	
				}
				if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
				{
					msg += u8"\n    所在群号: " + groupDatas[i].readGroupNumber();
					for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
					{
						if(groupDatas[i].readUserQQs(j) == userQQ)
						{
							msg += u8"\n    用户QQ: [CQ:at,qq=" + groupDatas[i].readUserQQs(j) + "]";
							xAuthToken = groupDatas[i].readUserXAuthTokens(j);
							successful = true;
							break;
						}
					}
				}
			}
			if(successful)
			{
				try
				{
					statusCode = cq::httpRequest::totalValidationCode(signInCode, xAuthToken);
					msg += u8"\n    返回：" + statusCode; // Message 类可以进行加法运算
				}
				catch(string errorString)
				{
					msg += u8"\n    错误提示：" + errorString;
				}
				catch(unsigned int statusCode)
				{
					msg += u8"\n    错误码：" + to_string(statusCode);
				}
			}
			else
			{
				msg += u8"\n    错误提示：NON-EXISTENT_USER";
			}
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			
            cq::message::send(e.target, msg); // 使用 message 命名空间的 send 函数
		}
		if(regex_match(e.raw_message,result,regexOtherSignIn))
		{
			//smatch::iterator x;
			string signInCode = result[1], userQQ = result[2], xAuthToken, statusCode;
			bool successful = false;

			cq::Message msg = u8"他人签到："; // string 到 Message 自动转换
			msg += u8"\n    签到码：" + signInCode; // Message 类可以进行加法运算

			for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
			{
				if(successful)
				{
					break;	
				}
				if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
				{
					msg += u8"\n    所在群号: " + groupDatas[i].readGroupNumber();
					for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
					{
						if(groupDatas[i].readUserQQs(j) == userQQ)
						{
							msg += u8"\n    用户QQ: [CQ:at,qq=" + groupDatas[i].readUserQQs(j) + "]";
							xAuthToken = groupDatas[i].readUserXAuthTokens(j);
							successful = true;
							break;
						}
					}
				}
			}
			if(successful)
			{
				try
				{
					statusCode = cq::httpRequest::totalValidationCode(signInCode, xAuthToken);
					msg += u8"\n    返回：" + statusCode; // Message 类可以进行加法运算
				}
				catch(string errorString)
				{
					msg += u8"\n    错误提示：" + errorString;
				}
				catch(unsigned int statusCode)
				{
					msg += u8"\n    错误码：" + to_string(statusCode);
				}
			}
			else
			{
				msg += u8"\n    错误提示：NON-EXISTENT_USER";
			}
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			
            cq::message::send(e.target, msg); // 使用 message 命名空间的 send 函数
		}
		if(regex_match(e.raw_message,result,regexMultiSignIn))
		{
			//multiSignIn(std::vector<cq::fileio::userDatas> userDatasVector, unsigned int groupIterator, unsigned int userIterator, std::string signInCode, std::vector<std::string>& returnState);
			string signInCode = result[1];
			vector<string> userQQ, xAuthTokens;
			vector<thread> signInThreads; /// 默认构造线程

			unsigned int userCounts;
			
			cq::Message msg = u8"多线程签到\n";

			for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
			{
				if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
				{
					userCounts = groupDatas[i].readTotalUserCounts();
					for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
					{
						xAuthTokens.push_back(groupDatas[i].readUserXAuthTokens(j));
						userQQ.push_back(groupDatas[i].readUserQQs(j));
					}
				}
			}

			try
			{
				for (unsigned int i = 0; i < userCounts; ++i) {
					cq::logging::debug(u8"Multy", u8"QQ：" + userQQ[i] + u8"Token: " + xAuthTokens[i]);
					signInThreads[i] = thread(cq::httpRequest::multiSignIn, userQQ[i], signInCode, xAuthTokens[i]);
				}
				for (unsigned int i = 0; i < userCounts; ++i) {
					signInThreads[i].detach();
					cq::logging::debug(u8"Multy", u8"Thread：" + to_string(i));
				}
			}
			catch (const cq::exception::ApiError &err)
			{
           		// API 调用失败
            	cq::logging::debug(u8"API", u8"调用失败，错误码：" + to_string(err.code));
        	}
			
			/*
			for (unsigned int i = 0; i < cq::fileio::userDatas::readTotalGroupCounts(); i++)
			{
				if(groupDatas[i].readGroupNumber() == to_string(e.group_id))
				{
					for (unsigned int j = 0; j < groupDatas[i].readTotalUserCounts(); j++)
					{
						msg += u8"    用户：[CQ:at,qq=" + groupDatas[i].readUserQQs(j) + "]，状态：" + returnState[j];
					}
				}
			}
			*/
			long long endRuntime = GetTickCount();
			msg += u8"\n执行耗时：" + to_string(endRuntime - startRuntime) + "ms";
			cq::message::send(e.target, msg);
		}

        //const auto memlist = cq::api::get_group_member_list(e.group_id); // 获取数据接口
        //cq::Message msg = u8"检测到精确消息，RawMessage="; // string 到 Message 自动转换
        //msg += std::to_string(memlist.size()) + u8" 个成员"; // Message 类可以进行加法运算
        //msg += e.raw_message; // Message 类可以进行加法运算
        //cq::message::send(e.target, msg); // 使用 message 命名空间的 send 函数
    };
	cq::app::on_coolq_exit = []
	{
		cq::logging::debug(u8"Exit", u8"Cool Q Exited!");
	};
}


// 添加菜单项，需要同时在 <appid>.json 文件的 menu 字段添加相应的条目，function 字段为 menu_demo_1
CQ_MENU(menu_management)
{
    cq::logging::info(u8"菜单", u8"点击了示例菜单1");
    try
	{
        cq::api::send_private_msg(administrator, "hello");
    }
	catch (const cq::exception::ApiError &)
	{
        cq::logging::warning(u8"菜单", u8"发送失败");
    }
}

// 不像 CQ_MAIN，CQ_MENU 可以多次调用来添加多个菜单
CQ_MENU(menu_settings)
{
    cq::logging::info(u8"菜单", u8"点击了示例菜单2");
    MessageBoxW(nullptr, L"这是一个提示", L"提示", MB_OK | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
}