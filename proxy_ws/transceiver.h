#include <memory>
#include <mutex>

#include "core.h"
#include "common/selector.h"
#include "common/socket.h"

namespace core {


class Transceiver : public Selectable {

protected:
	Socket socket;
	std::mutex send_mutex;
	uint8_t protocol_version = 0;

private:
	Buffer recv_buf;

protected:
	Transceiver() { }
	explicit Transceiver(Socket &&socket);

protected:
	void selected(Selector &selector, Selector::Flags flags) noexcept override;

	void send_message(const void *msg, size_t n);
	void send_message(const Sink::BufferPointer bufs[], size_t count);
	void send_message(std::initializer_list<Sink::BufferPointer> bufs) { return this->send_message(bufs.begin(), bufs.size()); }

	size_t receive_message(const struct NegotiateProtocolVersion &msg, size_t n);

	virtual void abort() noexcept;
	virtual void release() noexcept = 0;
	virtual size_t receive_message(const void *buf, size_t n) = 0;

};


} // namespace core
