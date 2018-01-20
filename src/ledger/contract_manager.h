/*
	bumo is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	bumo is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with bumo.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONTRACT_MANAGER_H_
#define CONTRACT_MANAGER_H_
#include <map>
#include <unordered_map>
#include <string>

#include <utils/headers.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <libplatform/libplatform-export.h>
#include <proto/cpp/chain.pb.h>
#include "ledgercontext_manager.h"

namespace bumo{

	class ContractParameter {
	public:
		ContractParameter();
		~ContractParameter();

		std::string code_;
		std::string input_;
		std::string this_address_;
		std::string sender_;
		std::string trigger_tx_;
		int32_t ope_index_;
		std::string consensus_value_;
		int64_t timestamp_;
		int64_t blocknumber_;
		LedgerContext *ledger_context_;
		int64_t pay_coin_amount_;
		protocol::Asset pay_asset_amount_;
	};

	class TestParameter{
	};

	class ContractTestParameter :public TestParameter {
	public:
		ContractTestParameter();
		~ContractTestParameter();

		bool exe_or_query_; //true: exe, false:query
		std::string contract_address_;
		std::string code_;
		std::string input_;
		std::string source_address_;
		int64_t contract_balance_;
		int64_t fee_;
	};

	class TransactionTestParameter :public TestParameter{
	public:
		TransactionTestParameter();
		~TransactionTestParameter();

		protocol::ConsensusValue consensus_value_;
	};

	class Contract {
	protected:
		int32_t type_;
		bool readonly_;
		int64_t id_;
		ContractParameter parameter_;

		Result result_;
		//int32_t error_code_;  //enum 0, FEE_NO_ENOUGH, MAX_TX
		//std::string error_msg_;
		int32_t tx_do_count_;  //transactions trigger by one contract
		utils::StringList logs_;
	public:
		Contract();
		Contract(bool readonly, const ContractParameter &parameter);
		virtual ~Contract();

	public:
		virtual bool Execute();
		virtual bool Cancel();
		virtual bool SourceCodeCheck();
		virtual bool Query(Json::Value& jsResult);

		int32_t GetTxDoCount();
		void IncTxDoCount();
		int64_t GetId();
		const ContractParameter &GetParameter();
		bool IsReadonly();
		const utils::StringList &GetLogs();
		void AddLog(const std::string &log);
		void SetResult(Result &result);
		Result &GetResult();
		static utils::Mutex contract_id_seed_lock_;
		static int64_t contract_id_seed_;

		enum TYPE {
			TYPE_V8 = 0,
			TYPE_ETH = 1
		};
	};

	class V8Contract : public Contract {
		v8::Isolate* isolate_;
		v8::Global<v8::Context> g_context_;
	public:
		V8Contract(bool readonly, const ContractParameter &parameter);
		virtual ~V8Contract();
	public:
		virtual bool Execute();
		virtual bool Cancel();
		virtual bool Query(Json::Value& jsResult);
		virtual bool SourceCodeCheck();

		static bool Initialize(int argc, char** argv);
		static bool LoadJsLibSource();
		static std::map<std::string, std::string> jslib_sources;
		static const std::string sender_name_;
		static const std::string this_address_;
		static const char* main_name_;
		static const char* query_name_;
		static const std::string trigger_tx_name_;
		static const std::string trigger_tx_index_name_;
		static const std::string this_header_name_;
		static const std::string pay_coin_amount_name_;
		static const std::string pay_asset_amount_name_;
		static const std::string block_timestamp_name_;
		static const std::string block_number_name_;

		static utils::Mutex isolate_to_contract_mutex_;
		static std::unordered_map<v8::Isolate*, V8Contract *> isolate_to_contract_;

		static v8::Platform* 	platform_;
		static v8::Isolate::CreateParams create_params_;

		static bool RemoveRandom(v8::Isolate* isolate, Json::Value &error_msg);
		static v8::Local<v8::Context> CreateContext(v8::Isolate* isolate, bool readonly);
		static V8Contract *GetContractFrom(v8::Isolate* isolate);
		static Json::Value ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);
		static const char* ToCString(const v8::String::Utf8Value& value);
		static void CallBackLog(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackGetAccountAsset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackSetValidators(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackGetValidators(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackPayCoin(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Include(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void InternalCheckTime(const v8::FunctionCallbackInfo<v8::Value>& args);
		//get a ledger info from a ledger
		static void CallBackGetLedgerInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
		//get transaction info from a transaction
		static void CallBackGetTransactionInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackContractQuery(const v8::FunctionCallbackInfo<v8::Value>& args);
		//static void CallBackGetThisAddress(const v8::FunctionCallbackInfo<v8::Value>& args);
		//make a transaction
		static void CallBackDoTransaction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static V8Contract *UnwrapContract(v8::Local<v8::Object> obj);
		static bool JsValueToCppJson(v8::Handle<v8::Context>& context, v8::Local<v8::Value>& jsvalue, Json::Value& jsonvalue);
		static bool CppJsonToJsValue(v8::Isolate* isolate, Json::Value& jsonvalue, v8::Local<v8::Value>& jsvalue);
		static void CallBackConfigFee(const v8::FunctionCallbackInfo<v8::Value>& args);


		//get balance of the given account 
		static void CallBackGetBalance(const v8::FunctionCallbackInfo<v8::Value>& args);
		//get the hash of one of the 1024 most recent complete blocks
		static void CallBackGetBlockHash(const v8::FunctionCallbackInfo<v8::Value>& args);

		//Sends a message with arbitrary date to a given address path
		static void CallBackStorageStore(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CallBackStorageDel(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetMetaData(const v8::FunctionCallbackInfo<v8::Value>& args, bool is_del = false);
		static void CallBackStorageLoad(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Get caller 
		static void CallBackGetCaller(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Get block number 
		static void CallBackBlockNumber(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Get block number 
		static void CallBackGetTxOrigin(const v8::FunctionCallbackInfo<v8::Value>& args);
		//selfDestruct

		//Get block timestamp 
		static void CallBackGetBlockTimestamp(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 add
		static void CallBackInt64Plus(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 sub
		static void CallBackInt64Sub(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 div
		static void CallBackInt64Div(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 mod
		static void CallBackInt64Mod(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 mod
		static void CallBackInt64Mul(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Int64 compare
		static void CallBackInt64Compare(const v8::FunctionCallbackInfo<v8::Value>& args);

	};

	class QueryContract : public utils::Thread{
		Contract *contract_;
		ContractParameter parameter_;
		Json::Value result_;
		bool ret_;
		utils::Mutex mutex_;
	public:
		QueryContract();
		~QueryContract();

		bool Init(int32_t type, const ContractParameter &paramter);
		virtual void Run();
		void Cancel();
		bool GetResult(Json::Value &result);
	};

// 	class TestContract : public utils::Thread {
// 		int32_t type_;
// 		ContractTestParameter parameter_;
// 
// 		Json::Value result_;
// 		bool ret_;
// 		LedgerContext ledger_context;
// 	public:
// 		TestContract();
// 		~TestContract();
// 
// 		bool Init(int32_t type, const ContractTestParameter &parameter);
// 		virtual void Run();
// 		void Cancel();
// 		bool GetResult(Json::Value &result);
// 	};

	typedef std::map<int64_t, Contract *> ContractMap;
	class ContractManager :
		public utils::Singleton<ContractManager>{
		friend class utils::Singleton<ContractManager>;

		utils::Mutex contracts_lock_;
		ContractMap contracts_;
	public:
		ContractManager();
		~ContractManager();

		bool Initialize(int argc, char** argv);
		bool Exit();

		Result Execute(int32_t type, const ContractParameter &paramter);
		bool Query(int32_t type, const ContractParameter &paramter, Json::Value &result);
		bool Cancel(int64_t contract_id);
		Result SourceCodeCheck(int32_t type, const std::string &code);
		//bool Test(int32_t type, const ContractTestParameter &paramter, Json::Value& jsResult);
		Contract *GetContract(int64_t contract_id);
	};
}
#endif