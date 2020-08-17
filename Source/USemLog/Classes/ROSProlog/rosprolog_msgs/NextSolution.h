// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "ROSBridgeSrv.h"

namespace rosprolog_msgs
{
	class NextSolution : public FROSBridgeSrv
	{

	public:
		NextSolution()
		{
			SrvType = TEXT("rosprolog/next_solution");
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
				return	TEXT("NextSolution::Request { id = ") + id + TEXT("} ");
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
			uint8 status;
			FString solution;

		public:
			Response() {}

			Response(uint8 InStatus) : status(InStatus) {}

			Response(uint8 InStatus, FString InSolution) {
				status = InStatus;
				solution = InSolution;
			}

			void SetStatus(uint8 S) { status = S;}

			bool GetStatus() { return status;}

			void SetSolution(FString S) { solution = S;}

			FString GetSolution() {	return solution;}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				status = JsonObject->GetIntegerField("status");
				solution = JsonObject->GetStringField("solution");
			}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response rep;
				rep.FromJson(JsonObject);
				return rep;
			}

			virtual FString ToString() const override
			{
				return TEXT("Command::Response { status = ") + FString::FromInt(status) + TEXT(", solution = ") + solution + TEXT(" } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetBoolField("status", status);
				Object->SetStringField("solution", solution);
				return Object;
			}
		};
	};
} // namespace rosprolog_msgs 



