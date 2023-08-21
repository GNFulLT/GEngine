#ifndef ITRANSFER_HANDLE_H
#define ITRANSFER_HANDLE_H

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

#endif // ITRANSFER_HANDLE_H