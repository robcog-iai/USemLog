// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Events/SLEvents.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
* Structure holding the parameters of creating the google charts
*/
struct FSLGoogleChartsParameters
{
	// Should include legend at the bottom
	uint8 bLegend : 1;

	// Use tooltips
	uint8 bTooltips : 1;

	// Overwrite if file already exists
	uint8 bOverwrite : 1;

	// Store start time
	float StartTime = -1.f;

	// Store end time
	float EndTime = -1.f;

	// Task id (Description)
	FString TaskId = "TaskId";

	// Episode id
	FString EpisodeId = "EpisodeId";

	// Events selection
	FLSymbolicEventsSelection EventsSelection;

	// Default constructor
	FSLGoogleChartsParameters() :
		bLegend(false),
		bTooltips(false)
	{};
};

/**
 * Function for exporting as html google charts
 */
struct FSLGoogleCharts
{
	// Write google charts timeline html page from the events
	static bool WriteTimelines(const TArray<TSharedPtr<ISLEvent>>& InEvents,
		const FString& DirectoryPath,
		const FString& InEpId,
		const FSLGoogleChartsParameters& Params = FSLGoogleChartsParameters())
	{
		FString FullFilePath = DirectoryPath + "/" + InEpId + TEXT("_TL.html");
		FPaths::RemoveDuplicateSlashes(FullFilePath);

		if (FPaths::FileExists(FullFilePath) && !Params.bOverwrite)
		{
			return false;
		}

		// Timeline boilerplate 
		FString TimelineStr =
			"<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\n"
			"\n"
			"<script type=\"text/javascript\">\n"
			"\t google.charts.load(\"current\", {packages:[\"timeline\"]});\n"
			"\t google.charts.setOnLoadCallback(drawChart);\n"
			"\t function drawChart() {\n"
			"\t\t var container = document.getElementById('event_tl');\n"
			"\t\t var chart = new google.visualization.Timeline(container);\n"
			"\t\t var dataTable = new google.visualization.DataTable();\n"
			"\n "
			"\t\t dataTable.addColumn({ type: 'string', id: 'context' });\n"
			"\t\t dataTable.addColumn({ type: 'string', id: 'event_id' });\n"
		;

		if (Params.bTooltips)
		{
			TimelineStr.Append("\t\t dataTable.addColumn({ type: 'string', role: 'tooltip', 'p': {'html': true} });\n");
		}

		TimelineStr.Append(
			"\t\t dataTable.addColumn({ type: 'number', id: 'start' });\n"
			"\t\t dataTable.addColumn({ type: 'number', id: 'end' });\n"
			"\n"
			"\t\t dataTable.addRows([\n"
			"\n"
		);

		// Add episode duration
		if (Params.StartTime >= 0 && Params.EndTime > 0)
		{
			const FString StartStr = FString::Printf(TEXT("%.3f"), Params.StartTime); 
			const FString EndStr = FString::Printf(TEXT("%.3f"), Params.EndTime); 
			const FString StartMsStr = FString::Printf(TEXT("%.3f"), Params.StartTime * 1000.f); //FString::SanitizeFloat(Ev->Start * 1000.f);
			const FString EndMsStr = FString::Printf(TEXT("%.3f"), Params.EndTime * 1000.f);  //FString::SanitizeFloat(Ev->End * 1000.f);
			
			TimelineStr.Append("\t\t [ \'" + Params.TaskId + "\' , \'" + Params.EpisodeId + "\' , ");
			if (Params.bTooltips)
			{
				TimelineStr.Append("emptyFunc(), ");
			}
			TimelineStr.Append(StartMsStr + " , " + EndMsStr + " ],\n");  // google charts needs millisecods
		}

		// Add event times
		for (const auto& Ev : InEvents)
		{
			if (!ShouldEventBeWritten(Ev, Params.EventsSelection))
			{
				continue;
			}

			const FString StartStr = FString::Printf(TEXT("%.3f"), Ev->StartTime); //FString::SanitizeFloat(Ev->Start);
			const FString EndStr = FString::Printf(TEXT("%.3f"), Ev->EndTime); //FString::SanitizeFloat(Ev->End);
			const FString StartMsStr = FString::Printf(TEXT("%.3f"), Ev->StartTime * 1000.f); //FString::SanitizeFloat(Ev->Start * 1000.f);
			const FString EndMsStr = FString::Printf(TEXT("%.3f"), Ev->EndTime * 1000.f);  //FString::SanitizeFloat(Ev->End * 1000.f);

			TimelineStr.Append("\t\t [ \'" + Ev->Context() + "\' , \'" + Ev->Id + "\' , " );
			if (Params.bTooltips)
			{
				TimelineStr.Append(
					"createTooltipHTMLContent("
					+ StartStr + ", "
					+ EndStr + ", "
					+ Ev->Tooltip() + "), " );
			}
			TimelineStr.Append(StartMsStr + " , " + EndMsStr + " ],\n");  // google charts needs millisecods
		}

		TimelineStr.Append(
			"\n"
			"\t\t]);\n"
			"\n"
			"\t\t var options = {\n"
			"\t\t\t timeline: {showRowLabels: true, colorByRowLabel: true},\n"
			"\t\t\t avoidOverlappingGridLines: false,\n"
			"\t\t\t tooltip: {isHtml: true}\n"
			"\t\t };\n"
			"\t\t chart.draw(dataTable, options);\n"
			"\t }\n");

		if (Params.bTooltips)
		{
			TimelineStr.Append(
				"\t function createTooltipHTMLContent(Start, End, Key1, Val1, Key2, Val2, Key3, Val3, Key4, Val4, Key5, Val5){\n"
				"\t\t return '<center>' +\n"
				"\t\t\t '<font size=\"3\">' +\n"
				"\t\t\t '<p><strong>Duration:</strong> ' + (End - Start).toFixed(3) + 's</p>' +\n"
				"\t\t\t '<p>' + Start + 's - ' + End + 's</p>' +\n"
				"\t\t\t '<hr/>' +\n"
				"\t\t\t '</font>' +\n"
				"\t\t\t '<font size=\"2\">' +\n"
				"\t\t\t '<p><strong>' + Key1 + ':</strong> ' + Val1 + '</p>' +\n"
				"\t\t\t '<p><strong>' + Key2 + ':</strong> ' + Val2 + '</p>' +\n"
				"\t\t\t '<hr/>' +\n"
				"\t\t\t '<p><strong>' + Key3 + ':</strong> ' + Val3 + '</p>' +\n"
				"\t\t\t '<p><strong>' + Key4 + ':</strong> ' + Val4 + '</p>' +\n"
				"\t\t\t '<hr/>' +\n"
				"\t\t\t '</font>' +\n"
				"\t\t\t '<font size=\"1\">' +\n"
				"\t\t\t '<p><strong>' + Key5 + ':</strong> ' + Val5 + '</p>' +\n"
				"\t\t\t '</font>' +\n"
				"\t\t\t '</center>' \n"
				"\t } \n");
		}

		TimelineStr.Append("\n\t function emptyFunc() { return } \n");

		TimelineStr.Append(	
			"</script>\n"
			"<div id=\"event_tl\" style=\"height:900px;\"></div>");

		if (Params.bLegend)
		{
			TimelineStr.Append(FSLGoogleCharts::GetLengend(InEvents));
		}

		// Write map to file
		return FFileHelper::SaveStringToFile(TimelineStr, *FullFilePath);
	}

private:

	// Table showing the legend of the symbols
	static FString GetLengend(const TArray<TSharedPtr<ISLEvent>>& InEvents)
	{
		FString Legend =
			"\n"
			"\n";
		return Legend;
	}

	// Should the event be written
	static bool ShouldEventBeWritten(TSharedPtr<ISLEvent> Event, const FLSymbolicEventsSelection& EventSelection)
	{
		if (!Event.IsValid())
		{
			return false;
		}
		if (EventSelection.bSelectAll)
		{
			return true;
		}
		/* Contact */
		else if (Event.Get()->TypeName().StartsWith("Contact"))
		{
			return EventSelection.bContact || EventSelection.bManipulatorContact;
		}
		/* SupportedBy */
		else if (Event.Get()->TypeName().StartsWith("SupportedBy"))
		{
			return EventSelection.bSupportedBy;
		}
		/* Reach + PreGrasp*/
		else if (Event.Get()->TypeName().StartsWith("Reach")
			|| Event.Get()->TypeName().StartsWith("PreGrasp")
			)
		{
			return EventSelection.bReachAndPreGrasp;
		}
		/* Grasp */
		else if (Event.Get()->TypeName().StartsWith("Grasp"))
		{
			return EventSelection.bGrasp;
		}
		/* PickAndPlace */
		else if (Event.Get()->TypeName().StartsWith("Slide")
			|| Event.Get()->TypeName().StartsWith("PickUp")
			|| Event.Get()->TypeName().StartsWith("Transport")
			|| Event.Get()->TypeName().StartsWith("PutDown")
			)
		{
			return EventSelection.bPickAndPlace;
		}
		
		UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown event %s, will be written anyhow.."),
			*FString(__FUNCTION__), __LINE__, *Event.Get()->ToString());
		return true;

		//// TODO switch to UPROPERTY pure dynamic_cast does not work without RTTI
		///* All events */
		//if (EventSelection.bSelectAll)
		//{
		//	return true;
		//}
		///* Contact */
		//else if (dynamic_cast<FSLContactEvent*>(Event.Get()))
		//{
		//	return EventSelection.bContact;
		//}
		///* Supported By */
		//else if (dynamic_cast<FSLSupportedByEvent*>(Event.Get()))
		//{
		//	return EventSelection.bSupportedBy;
		//}
		///* Reach */
		//else if (dynamic_cast<FSLReachEvent*>(Event.Get()))
		//{
		//	return EventSelection.bReachAndPreGrasp;
		//}
		///* Grasp */
		//else if (dynamic_cast<FSLGraspEvent*>(Event.Get()))
		//{
		//	return EventSelection.bGrasp;
		//}
		///* PickAndPlace */
		//else if (dynamic_cast<FSLSlideEvent*>(Event.Get()))
		//{
		//	return EventSelection.bPickAndPlace;
		//}
		//else if (dynamic_cast<FSLPickUpEvent*>(Event.Get()))
		//{
		//	return EventSelection.bPickAndPlace;
		//}
		//else if (dynamic_cast<FSLTransportEvent*>(Event.Get()))
		//{
		//	return EventSelection.bPickAndPlace;
		//}
		//else if (dynamic_cast<FSLPutDownEvent*>(Event.Get()))
		//{
		//	return EventSelection.bPickAndPlace;
		//}

		//UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown event %s.."), *FString(__FUNCTION__), __LINE__, *Event.Get()->ToString());
		//return false;
	}
};
