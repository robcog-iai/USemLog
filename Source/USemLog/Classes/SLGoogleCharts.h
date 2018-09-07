#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
 * Function for exporting as html google charts
 */
struct FSLGoogleCharts
{
	// Write google charts timeline html page from the events
	static bool WriteTimelines(const TArray<TSharedPtr<ISLEvent>>& InEvents,
		const FString& InLogDir,
		const FString& InEpId,
		bool bIncludeLegend = true)
	{
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
			"\t\t dataTable.addColumn({ type: 'string', id: 'EventType' });\n"
			"\t\t dataTable.addColumn({ type: 'string', id: 'EventId' });\n"
			"\t\t dataTable.addColumn({ type: 'number', id: 'Start' });\n"
			"\t\t dataTable.addColumn({ type: 'number', id: 'End' });\n"
			"\n"
			"\t\t dataTable.addRows([\n"
			"\n";

		// Add event times
		for (const auto& Ev : InEvents)
		{
			TimelineStr.Append(
				"\t\t [ \'" + Ev->Context()+ "\' , \'"
				+ Ev->Id + "\' , "
				+ FString::SanitizeFloat(Ev->Start * 1000.f) + " , " // google charts needs millisecods
				+ FString::SanitizeFloat(Ev->End * 1000.f) + " ],\n" // google charts needs millisecods
			);
		}

		TimelineStr.Append(
			"\n"
			"\t\t]);\n"
			"\n"
			"\t\t var options = {\n"
			"\t\t\t timeline: { showRowLabels: false }\n"
			"\t\t };\n"
			"\n"
			"\t\t chart.draw(dataTable, options);\n"
			"\t}\n"
			"</script>\n"
			"<div id=\"event_tl\" style=\"height:900px;\"></div>"
		);

		if (bIncludeLegend)
		{
			TimelineStr.Append(FSLGoogleCharts::GetLengend(InEvents));
		}

		// Write map to file
		FString FullFilePath = FPaths::ProjectDir() +
			InLogDir + TEXT("/Episodes/") + InEpId + TEXT("_TL.html");
		FPaths::RemoveDuplicateSlashes(FullFilePath);
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
};
