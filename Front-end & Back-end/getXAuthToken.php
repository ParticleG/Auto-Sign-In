<?php

	function tocurl_get_body($url, $header) {
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, false);  //以get方式发送请求
		$response = curl_exec($ch);
		curl_close($ch);
		return $response;
	}
				
				
	function tocurl_get_state($url, $header) {
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, false);  //以get方式发送请求
		curl_setopt($ch, CURLOPT_NOBODY, true);
		$data = curl_exec($ch);
		$response = curl_getinfo($ch,CURLINFO_HTTP_CODE); //返回http码
		curl_close($ch);
		return $response;
	}

	function tocurl_get_header($url, $header) {
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, false);  //以get方式发送请求
		curl_setopt($ch, CURLOPT_HEADER, true);
		curl_setopt($ch, CURLOPT_NOBODY, true);
		$data = curl_exec($ch);
		$headerSize = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
		$reheader = substr($data, 0, $headerSize);
		curl_close($ch);
		return $reheader;
	}

	header('Content-type:text/json');
	$returnMessage = array("code" => 0, "message" => "");
	$account = $_REQUEST["account"];
	$password = urlencode(urlencode($_REQUEST["password"]));
	$url_get_access_token = 'https://mobile.yiban.cn/api/v2/passport/login?account='.(string)$account.'&passwd='.(string)$password.'&ct=2&app=1&v=4.6.2&apn=3gnet&identify=867520044297975&sig=316ab653300d5108&token=&device=OnePlus%3AONEPLUS+A6003&sversion=28';
	$header_get_access_token = array('Authorization: Bearer','loginToken:','AppVersion: 4.6.2','Connection: Keep-Alive');
	$return_body = json_decode((tocurl_get_body($url_get_access_token, $header_get_access_token)), true);
	if ($return_body["response"] != 100) {
		$returnMessage["message"] = "未能使用提供的账号密码登入易班，易班的返回内容为：".$return_body["message"];
		exit(json_encode($returnMessage));
	}
	$access_token = $return_body["data"]["access_token"];
	$url_get_verify_request = 'https://f.yiban.cn/iapp/index?act=iapp319528&v='.(string)$access_token;
	$header_get_verify_request = array('Connection: keep-alive','Cache-Control: no-cache','Upgrade-Insecure-Requests: 1','Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8','Referer: https://oauth.yiban.cn/code/html?client_id=dc76b1d4eba657b0&redirect_uri=http://f.yiban.cn/iapp319528','Accept-Encoding: gzip, deflate','Accept-Language: zh-CN,en-US;q=0.9','Cookie: client=android; _YB_OPEN_V2_0=r2130lIb0h1env80; yibanM_user_token='.$access_token,'X-Requested-With: com.yiban.app');
	$return_header = tocurl_get_header($url_get_verify_request, $header_get_verify_request);
	$verify_request = strstr($return_header, "verify_request=", false);
	$si = strpos($verify_request, "&");
	$verify_request = substr($verify_request, 0, $si);
	$url_get_x_auth_token = 'http://skl.hdu.edu.cn/api/login/yiban?'.$verify_request;
	$return_header = get_headers($url_get_x_auth_token);
	if (!strstr($return_header[0], "302")) {
		$returnMessage["message"] = "未能获取XAuthToken";
		exit(json_encode($returnMessage));
	}
	$returnMessage["code"] = 1;
	$returnMessage["message"] = substr($return_header[4], 14, 37);
	echo json_encode($returnMessage);

?>