#ifndef ITRANSFER_OP_H
#define ITRANSFER_OP_H

#include <expected>

#include "engine/rendering/vulkan/transfer/itransfer_handle.h"



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