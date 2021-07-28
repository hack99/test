#ifndef MSG_BASE_H
#define MSG_BASE_H

class msg_base
{
	enum { max_buffer_length = 8192 };
	typedef unsigned short length_holder_t;
public:
	class length_holder
	{
	};
public:
	msg_base(unsigned int size) : buffer_((char*)malloc(max_buffer_length)), size_(size), rd_ptr_(0), wr_ptr_(0), length_holder_(NULL)
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
		if (length_holder_) *length_holder_ = wr_ptr_;
	}
	
	void write_2(const void* data)
	{
		*(short*)(buffer_+wr_ptr_) = *(short*)data;
		wr_ptr_+=2;
		if (length_holder_) *length_holder_ = wr_ptr_;
	}

	void write_4(const void* data)
	{
		*(int*)(buffer_+wr_ptr_) = *(int*)data;
		wr_ptr_+=4;
		if (length_holder_) *length_holder_ = wr_ptr_;
	}

	void write_8(const void* data)
	{
		*(double*)(buffer_+wr_ptr_) = *(double*)data;
		wr_ptr_+=8;
		if (length_holder_) *length_holder_ = wr_ptr_;
	}

	void write_data(const void* data, unsigned int len)
	{
		memcpy(buffer_+wr_ptr_, data, len);
		wr_ptr_+=len;
		if (length_holder_) *length_holder_ = wr_ptr_;
	}

	msg_base& operator<<(length_holder data)
	{
		if (length_holder_ != NULL)
		{
			// Only one place holder
			return *this;
		}
		length_holder_ = (length_holder_t*)(buffer_+wr_ptr_);
		wr_ptr_ += sizeof(length_holder_t);
		*length_holder_ = wr_ptr_;
		return *this;
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

	void dump()
	{
		for (int i = rd_ptr_; i < wr_ptr_; i++)
		{
			printf("%02X ", (unsigned char)buffer_[i]);
		}
		printf("\n");
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
	length_holder_t* length_holder_;
};

#endif // MSG_BASE_H
