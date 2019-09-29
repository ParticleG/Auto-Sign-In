<?php 
	$sign_in_code = $_GET['sign-in-code'];
	$x_auth_token = $_GET['x-auth-token'];
	$run_times = 1;
	
	do
	{
		$right_answer = true;
		
		$url_get = 'https://skl.hdu.edu.cn/api/checkIn/code-check-in?code='.$sign_in_code;
		$header_get = array('Host: skl.hdu.edu.cn','Connection: keep-alive','Pragma: no-cache','Cache-Control: no-cache','Accept: application/json, text/plain, */*','X-Auth-Token:'.$x_auth_token,'User-Agent: Mozilla/5.0 (Linux; Android 8.1.0; ONEPLUS A6000 Build/OPM6.171019.030.H1; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/72.0.3626.121 Mobile Safari/537.36 yiban_android','Referer: http://skl.hdu.edu.cn/?type=2','Accept-Encoding: gzip, deflate','Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7','Cookie: client=android','X-Requested-With: com.yiban.app');
		//$content = array('name' => 'fdipzone');  //无内容
		//httpStatus(102);
		$auth_token_check = tocurl_get_2($url_get, $header_get);
		if($auth_token_check == 403)
		{
			echo "授权码错误！";
			break;
		}
		$image_data_byte = tocurl_get($url_get, $header_get);
		
		//$data = json_decode($response, true);
		
		$base64_image = chunk_split(base64_encode($image_data_byte)); 
		//$base64_image = str_replace("\n","",$base64_image);
		//$base64_image = urlencode($base64_image);
		
		$url_post = 'http://www.damagou.top/apiv1/recognize.html';
		$header_post = array('Accept: */*');

		$content_post = array(
			'image'=>$base64_image,
			'userkey'=>'737a6f3402df4b196aa956566325a5fb',
			'type'=>'1002'
		); 
		//$content_post =  json_encode($content_post);
		
		//var_dump($content_post);
		
		$result = tocurl_post($url_post, $header_post, $content_post);
		
		//echo $result;
		
		$url_get = 'https://skl.hdu.edu.cn/api/checkIn/valid-code?code='.$result;
		$header_get = array('Host: skl.hdu.edu.cn','Connection: keep-alive','Pragma: no-cache','Cache-Control: no-cache','Accept: application/json, text/plain, */*','X-Auth-Token: '.$x_auth_token,'User-Agent: Mozilla/5.0 (Linux; Android 8.1.0; ONEPLUS A6000 Build/OPM6.171019.030.H1; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/72.0.3626.121 Mobile Safari/537.36 yiban_android','Referer: http://skl.hdu.edu.cn/?type=2','Accept-Encoding: gzip, deflate','Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7','Cookie: client=android','X-Requested-With: com.yiban.app');
		//$content = array('name' => 'fdipzone');  //无内容
		
		$sign_result = tocurl_get_2($url_get, $header_get);
		
		//echo $sign_result;
		
		if($sign_result == 401)
		{
			echo "签到码错误或失效！".PHP_EOL;
			echo "签到码：".$sign_in_code.PHP_EOL;
			echo "验证码：".$result.PHP_EOL;
			echo "尝试次数：".$run_times;
		}
		else
		{
			if($sign_result == 400)
			{
				if($run_times > 10)
				{
					echo "尝试次数超过10次！请检查程序！";
				}
				else
				{
					$run_times = $run_times + 1;
					$right_answer = false;
				}
			}
			else
			{
				if($sign_result == 200)
				{
					echo "签到成功！".PHP_EOL;
				echo "签到码：".$sign_in_code.PHP_EOL;
				echo "验证码：".$result.PHP_EOL;
				echo "尝试次数：".$run_times;
				}
				else
				{
					echo "未知错误！请检查程序！";
				}
			}
		}
	}while($right_answer == false);
				
				
	function tocurl_get($url, $header)
	{
		$ch = curl_init();
					
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, false);  //以get方式发送请求
		$response = curl_exec($ch);
	
		curl_close($ch);
		return $response;
	}
				
	function tocurl_post($url, $header, $content)
	{
		$ch = curl_init();
					
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, true);  //以post方式发送请求
		curl_setopt($ch, CURLOPT_POSTFIELDS, $content);
		$response = curl_exec($ch);
					
		curl_close($ch);
		return $response;
	}
				
	function tocurl_get_2($url, $header)
	{
		$ch = curl_init();
		
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
		curl_setopt($ch, CURLOPT_POST, false);  //以get方式发送请求
		$data = curl_exec($ch);
		$response = curl_getinfo($ch,CURLINFO_HTTP_CODE); //返回http码
		
		curl_close($ch);
		return $response;
	}
	function httpStatus($num){//网页返回码
        static $http = array (
            100 => "HTTP/1.1 100 Continue",
            101 => "HTTP/1.1 101 Switching Protocols",
			102 => "HTTP/1.1 102 Processing",
            200 => "HTTP/1.1 200 OK",
            201 => "HTTP/1.1 201 Created",
            202 => "HTTP/1.1 202 Accepted",
            203 => "HTTP/1.1 203 Non-Authoritative Information",
            204 => "HTTP/1.1 204 No Content",
            205 => "HTTP/1.1 205 Reset Content",
            206 => "HTTP/1.1 206 Partial Content",
            300 => "HTTP/1.1 300 Multiple Choices",
            301 => "HTTP/1.1 301 Moved Permanently",
            302 => "HTTP/1.1 302 Found",
            303 => "HTTP/1.1 303 See Other",
            304 => "HTTP/1.1 304 Not Modified",
            305 => "HTTP/1.1 305 Use Proxy",
            307 => "HTTP/1.1 307 Temporary Redirect",
            400 => "HTTP/1.1 400 Bad Request",
            401 => "HTTP/1.1 401 Unauthorized",
            402 => "HTTP/1.1 402 Payment Required",
            403 => "HTTP/1.1 403 Forbidden",
            404 => "HTTP/1.1 404 Not Found",
            405 => "HTTP/1.1 405 Method Not Allowed",
            406 => "HTTP/1.1 406 Not Acceptable",
            407 => "HTTP/1.1 407 Proxy Authentication Required",
            408 => "HTTP/1.1 408 Request Time-out",
            409 => "HTTP/1.1 409 Conflict",
            410 => "HTTP/1.1 410 Gone",
            411 => "HTTP/1.1 411 Length Required",
            412 => "HTTP/1.1 412 Precondition Failed",
            413 => "HTTP/1.1 413 Request Entity Too Large",
            414 => "HTTP/1.1 414 Request-URI Too Large",
            415 => "HTTP/1.1 415 Unsupported Media Type",
            416 => "HTTP/1.1 416 Requested range not satisfiable",
            417 => "HTTP/1.1 417 Expectation Failed",
            500 => "HTTP/1.1 500 Internal Server Error",
            501 => "HTTP/1.1 501 Not Implemented",
            502 => "HTTP/1.1 502 Bad Gateway",
            503 => "HTTP/1.1 503 Service Unavailable",
            504 => "HTTP/1.1 504 Gateway Time-out"
        );
        header($http[$num]);
        exit();
    }
?> 