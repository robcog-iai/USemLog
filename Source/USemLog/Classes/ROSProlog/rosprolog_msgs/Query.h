// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "ROSBridgeSrv.h"

namespace rosprolog_msgs
{
	class Query : public FROSBridgeSrv
	{

	public:
		Query()
		{
			SrvType = TEXT("rosprolog/query");
		}

		class Request : public SrvRequest
		{
		private:
			// Add your dfferent Request parameters here
			uint8 mode;
			FString query;
			FString id;
		public:
			Request() {}

			Request(uint8 InMode,	FString InQuery, FString InID):
				mode(InMode),
				query(InQuery),
				id(InID) { }

			uint8 GetMode() const {	return mode;}
			FString GetQuery() const { return query;}
			FString GetID() const {	return id;}

			void SetMode(uint8 InMode) { mode = InMode;}
			void SetQuery(FString InQuery) { query = InQuery;}
			void SetID(FString InID) { id = InID;}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				mode = JsonObject->GetIntegerField(TEXT("mode"));
				query = JsonObject->GetStringField(TEXT("query"));
				id = JsonObject->GetStringField(TEXT("id"));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return	TEXT("Command::Request { mode = ") + FString::FromInt(mode) \
						+ TEXT(", query = ") + query								\
						+ TEXT(", id = ") + id + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetNumberField(TEXT("mode"), mode);
				Object->SetStringField(TEXT("query"), query);
				Object->SetStringField(TEXT("id"), id);
				return Object;
			}
		};

		class Response : public SrvResponse
		{
		private:
			// Add your dfferent Response parameters here
			bool Success;
			FString Message;

		public:
			Response() {
				Success = false;
			}

			Response(bool InSuccess) : Success(InSuccess) {}

			Response(bool InSuccess, FString InMsg) {
				Success = InSuccess;
				Message = InMsg;
			}

			void SetSuccess(bool S)
			{
				Success = S;
			}

			bool GetSuccess()
			{
				return Success;
			}

			void SetMessage(FString S)
			{
				Message = S;
			}

			FString GetMessage()
			{
				return Message;
			}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				Success = JsonObject->GetBoolField("ok");
				Message = JsonObject->GetStringField("message");
			}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response rep; 
				rep.FromJson(JsonObject);
				return rep;
			}

			virtual FString ToString() const override
			{
				return TEXT("Query::Response { ok = ") + FString::FromInt( (int)Success) + TEXT(", message = ") + Message + TEXT(" } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetBoolField("ok", Success);
				Object->SetStringField("message", Message);
				return Object;
			}
		};
	};
} // namespace rosprolog_msgs 



