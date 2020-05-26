#include "transceiver.h"

#include <cassert>

#include "common/log.h"

extern Log elog;

namespace core {


Transceiver::Transceiver(Socket &&socket) : socket(std::move(socket)) {
	struct linger linger { .l_onoff = true, .l_linger = 0 };
	this->socket.setsockopt(SOL_SOCKET, SO_LINGER, linger);
}

void Transceiver::selected(Selector &selector, Selector::Flags flags __unused) noexcept {
	assert((flags & Selector::Flags::READABLE) != Selector::Flags::NONE);
	try {
		for (;;) {
			recv_buf.ensure(recv_buf.ppos() + 1460);
			ssize_t r = socket.recv(recv_buf.pptr, recv_buf.prem(), MSG_DONTWAIT);
			if (r < 0) {
				break;
			}
			if (r == 0) {
				selector.modify(socket, this, Selector::Flags::READABLE);
				return;
			}
			recv_buf.pptr += r;
			for (size_t n; (r = recv_buf.grem()) > 0 && (n = this->receive_message(recv_buf.gptr, r)) != 0;) {
				recv_buf.gptr += n;
				assert(recv_buf.gptr <= recv_buf.pptr);
			}
			recv_buf.compact();
		}
	}
	catch (const std::exception &e) {
		if (elog.error_enabled()) {
			elog.error() << "exception while processing message" << ": " << e.what() << std::endl;
		}
	}
	catch (...) {
		if (elog.error_enabled()) {
			elog.error() << "exception while processing message" << std::endl;
		}
	}
	this->release();
}

void Transceiver::send_message(const void *msg, size_t n) {
	std::lock_guard<std::mutex> lock(send_mutex);
	socket.write_fully(msg, n);
	socket.flush();
}

void Transceiver::send_message(const Sink::BufferPointer bufs[], size_t count) {
	std::lock_guard<std::mutex> lock(send_mutex);
	socket.write_fully(bufs, count);
	socket.flush();
}

size_t Transceiver::receive_message(const struct NegotiateProtocolVersion &msg, size_t n) {
	if (n < sizeof msg) {
		return 0;
	}
	if (msg.min_version > PROTOCOL_VERSION) {
		throw std::ios_base::failure("unsupported protocol");
	}
	protocol_version = std::min(msg.max_version, PROTOCOL_VERSION);
	return sizeof msg;
}

void Transceiver::abort() noexcept {
	try {
		socket.shutdown();
	}
	catch (...) {
	}
}


} // namespace core
