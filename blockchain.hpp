#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "transaction.hpp"
#include "neighbor.hpp"
#include <leveldb/db.h>
#include <map>
#include <set>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/string.hpp>
#include "json.hpp"

struct BlockHeaders {
	uint32_t version;
	std::string previous_hash;
	std::string transactions_hash;
	std::string target;
	std::string beneficiary;
	uint32_t nonce;
	BlockHeaders();
	BlockHeaders(uint32_t version
			, std::string previous_hash
			, std::string transactions_hash
			, std::string target, std::string beneficiay
			, uint32_t);
	BlockHeaders(std::string serialized_headers);
	std::string serialize();
	std::string hash();
};

struct Block {
	Block() {}
	Block(std::string serialized_block);
	std::string getMerkleRoot();
	std::string serialize();
	bool isValid(const std::string &target);

	// 會更新 world_state 以及 all_txs
	bool countWorldState(struct Blockchain &blockchain);

	BlockHeaders headers;
	int height;
	std::vector<Transaction> txs;
	std::map<std::string, uint64_t> world_state;
	std::set<std::string> all_txs;              // 從該區塊向前追溯的所有交易，以 sign 表示

	template<class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		archive & headers.version;
		archive & headers.previous_hash;
		archive & headers.transactions_hash;
		archive & headers.target;
		archive & headers.beneficiary;
		archive & headers.nonce;
		archive & height;
		archive & txs;
		archive & all_txs;
		archive & world_state;
	}
};

struct Blockchain {
	std::string target;
	std::string beneficiary;
	std::string public_key;
	std::string private_key;
	uint64_t fee;
	leveldb::DB* db;
	Neighbors neighbors;
	std::vector<Transaction> transaction_pool;

	Blockchain(nlohmann::json config);

	int getBlockCount();
	void broadcastBlock(Block block);
	void broadcastTransaction(Transaction tx);
	void addBlock(Block block);
	void saveBlock(std::string hash, Block block);
	Block getBlock(std::string block_hash);
	void mining();
	void initDb();
	unsigned int getBalance(std::string address);
	void showWorldState();
	Block getLatestBlock();
	void sendToAddress(std::string address, uint64_t amount);
	bool addRemoteTransaction(std::string tx_str);
};
#endif
