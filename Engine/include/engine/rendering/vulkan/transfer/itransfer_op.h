#ifndef ITRANSFER_OP_H
#define ITRANSFER_OP_H

#include <expected>

class GVulkanCommandBuffer;

enum TRANSFER_QUEUE_GET_ERR
{
	TRANSFER_QUEUE_GET_ERR_TIMEOUT
};

class ITransferHandle
{
public:
	virtual ~ITransferHandle() = default;

	virtual GVulkanCommandBuffer* get_command_buffer() = 0;
private:
};

class ITransferOperations
{
public:

	virtual ~ITransferOperations() = default;

	virtual bool init() = 0;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() = 0;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) = 0;

	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) = 0;

	virtual void destroy() = 0;
private:

};

#endif // ITRANSFER_OP_H