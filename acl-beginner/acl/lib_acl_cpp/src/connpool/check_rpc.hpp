#pragma once
#include "acl_cpp/acl_cpp_define.hpp"
#include "acl_cpp/ipc/rpc.hpp"

namespace acl {

class connect_monitor;
class check_client;

class check_rpc : public rpc_request {
public:
	check_rpc(connect_monitor& monitor, check_client& checker);

protected:
	// 子线程处理函数
	// @override
	void rpc_run();

	// 主线程处理过程，收到子线程任务完成的消息
	// @override
	void rpc_onover();

private:
	connect_monitor& monitor_;
	check_client& checker_;

	~check_rpc();
};

} // namespace acl
