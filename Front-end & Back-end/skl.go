package controllers

import (
	"crypto/tls"
	"net/http"
	"net/url"

	"github.com/astaxie/beego"
	"github.com/kirinlabs/HttpRequest"
	"github.com/tidwall/gjson"
)

// SKLXAuthTokenController ...
type SKLXAuthTokenController struct {
	beego.Controller
}

// ReturnMessage ...
type ReturnMessage struct {
	Code    int
	Message string
}

// Get ...
func (c *SKLXAuthTokenController) Get() {
	returnMessage := &ReturnMessage{0, ""}
	account := c.GetString("account")
	password := c.GetString("password")
	password = url.QueryEscape(url.QueryEscape(password))
	req := HttpRequest.NewRequest()
	req.SetTLSClient(&tls.Config{InsecureSkipVerify: true})
	req.SetHeaders(map[string]string{
		"Authorization": "Bearer",
		"AppVersion":    "4.6.2",
	})
	res, err := req.Get("https://mobile.yiban.cn/api/v2/passport/login?account=" + account + "&passwd=" + password + "&ct=2&app=1&v=4.6.2&apn=3gnet&identify=867520044297975&sig=316ab653300d5108&token=&device=OnePlus%3AONEPLUS+A6003&sversion=28")
	if err != nil {
		returnMessage.Message = "登录请求错误：" + string(err.Error())
		c.Data["json"] = returnMessage
		c.ServeJSON()
		return
	}
	body, err := res.Body()
	if err != nil {
		returnMessage.Message = "登录易班失败，未能获取请求body！"
		c.Data["json"] = returnMessage
		c.ServeJSON()
		return
	}
	bodyString := string(body)
	if code := gjson.Get(bodyString, "response").String(); code != "100" {
		returnMessage.Message = "未能使用提供的账号密码登录易班，易班的返回内容为：" + gjson.Get(bodyString, "message").String()
		c.Data["json"] = returnMessage
		c.ServeJSON()
		return
	}
	accessToken := gjson.Get(bodyString, "data.access_token").String()
	returnMessage.Code = 1
	returnMessage.Message = accessToken
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}
	client := &http.Client{
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
		Transport: tr,
	}
	req2, _ := http.NewRequest("GET", "https://f.yiban.cn/iapp/index?act=iapp319528&v="+accessToken, nil)
	req2.Header.Add("Upgrade-Insecure-Requests", "1")
	req2.Header.Add("Referer", "https://oauth.yiban.cn/code/html?client_id=dc76b1d4eba657b0&redirect_uri=http://f.yiban.cn/iapp319528")
	req2.Header.Add("X-Requested-With", "com.yiban.app")
	req2.Header.Add("Cookie", "client=android; _YB_OPEN_V2_0=r2130lIb0h1env80; yibanM_user_token="+accessToken)
	res2, err2 := client.Do(req2)
	if err2 != nil {
		returnMessage.Message = "获取verify_request请求错误：" + string(err2.Error())
		c.Data["json"] = returnMessage
		c.ServeJSON()
		return
	}
	defer res2.Body.Close()
	location := res2.Header.Get("Location")
	returnMessage.Message = location
	returnMessage.Code = 1
	res2, err2 = client.Get(location)
	if err2 != nil {
		returnMessage.Message = "获取X-Auth-Token请求错误：" + string(err2.Error())
		c.Data["json"] = returnMessage
		c.ServeJSON()
		return
	}
	defer res2.Body.Close()
	returnMessage.Message = res2.Header.Get("X-Auth-Token")
	c.Data["json"] = returnMessage
	c.ServeJSON()
}
