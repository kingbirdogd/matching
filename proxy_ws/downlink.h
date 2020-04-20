#include <shared_mutex>
#include <thread>
#include "state.h"
#include "transceiver.h"
#include "common/log.h"

extern Log elog;


namespace state {


class Downlink : public core::Transceiver {

private:
	static std::shared_mutex all_downlinks_mutex;
	static std::list<Downlink *> all_downlinks;

public:
	template <typename M>
	static void broadcast(const M &msg) {
		if (elog.trace_enabled()) {
            std::thread::id this_id = std::this_thread::get_id();
            elog.trace() << "Thread(" << this_id << ") !! " << msg << std::endl;
		}
    else if (elog.debug_enabled()) {
      elog.debug() << " !! " << msg << std::endl;
    }
		broadcast(UINT64_C(1) << (msg.opcode & ~0xC0), &msg, message_size<M>()(&msg, sizeof msg));
	}

	static void broadcast(uint64_t notification_flag, const void *msg, size_t n);
	static void broadcast(uint64_t notification_flag, const Sink::BufferPointer bufs[], size_t count);
	static void broadcast(uint64_t notification_flag, std::initializer_list<Sink::BufferPointer> bufs) { return broadcast(notification_flag, bufs.begin(), bufs.size()); }

	static void broadcast(uint64_t notification_mask, uint64_t notification_flags, const void *msg, size_t n);

protected:
	const sockaddr_in6 peer_addr;
	uint64_t notification_mask = 0;

private:
	std::list<Downlink *>::iterator all_downlinks_itr;

public:
	Downlink(Socket &&socket, const sockaddr_in6 &peer_addr, uint8_t min_protocol_version = 7, uint8_t max_protocol_version = PROTOCOL_VERSION);
	~Downlink();

public:
	using Transceiver::send_message;

protected:
	size_t receive_message(const struct SetNotificationMask &msg, size_t n);

private:
	void send_order(const OrderRecord &order);

};


} // namespace state
