// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "ROSBridgeSrv.h"

namespace rosprolog_msgs
{
	class Finish : public FROSBridgeSrv
	{

	public:
		Finish()
		{
			SrvType = TEXT("rosprolog/finish");
		}

		class Request : public SrvRequest
		{
		private:
			// Add your dfferent Request parameters here
			FString id;
		public:
			Request() {}

			Request(FString InID) :
				id(InID) { }

			FString GetID() const { return id; }

			void SetID(FString InID) { id = InID; }

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
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
				return	TEXT("Finish::Request { id = ") + id + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetStringField(TEXT("id"), id);
				return Object;
			}
		};

		class Response : public SrvResponse
		{
		private:
			// Add your dfferent Response parameters here

		public:
			Response() {}


			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override {}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject) {
				Response rep;
				rep.FromJson(JsonObject);
				return rep;
			}

			virtual FString ToString() const override
			{
				return TEXT("Finish::Response {} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};
	};
} // namespace rosprolog_msgs 