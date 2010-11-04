#ifndef MSG_BASE_H
#define MSG_BASE_H

class msg_base
{
	enum { max_buffer_length = 8192 };
public:
	msg_base(unsigned int size) : buffer_((char*)malloc(max_buffer_length)), size_(size), rd_ptr_(0), wr_ptr_(0)
	{
		std::cout << "msg_base::msg_base(" << size << ")" << std::endl;
	}
	virtual ~msg_base()
	{
		std::cout << "msg_base::~msg_base" << std::endl;
		free(buffer_);
	}

	void write_1(const void* data)
	{
		*(char*)(buffer_+wr_ptr_) = *(char*)data;
		wr_ptr_++;
	}
	
	void write_2(const void* data)
	{
		*(short*)(buffer_+wr_ptr_) = *(short*)data;
		wr_ptr_+=2;
	}

	void write_4(const void* data)
	{
		*(int*)(buffer_+wr_ptr_) = *(int*)data;
		wr_ptr_+=4;
	}

	void write_8(const void* data)
	{
		*(double*)(buffer_+wr_ptr_) = *(double*)data;
		wr_ptr_+=8;
	}

	void write_data(const void* data, unsigned int len)
	{
		memcpy(buffer_+wr_ptr_, data, len);
		wr_ptr_+=len;
	}

	msg_base& operator<<(unsigned short data)
	{
		write_2((void*)&data);
		return *this;
	}

	msg_base& operator<<(short data)
	{
		write_2((void*)&data);
		return *this;
	}

	msg_base& operator<<(const std::string& data)
	{
		write_data(data.c_str(), data.length()+1);
		return *this;
	}

	msg_base& operator<<(const char* data)
	{
		write_data(data, strlen(data)+1);
		return *this;
	}

	char* const buffer() { return buffer_; }
	const unsigned int size() { return size_; }
	int& rd_ptr() { return rd_ptr_; }
	int& wr_ptr() { return wr_ptr_; }
protected:
	char *const buffer_;
	const unsigned int size_;
	int rd_ptr_;
	int wr_ptr_;
};

#endif // MSG_BASE_H
