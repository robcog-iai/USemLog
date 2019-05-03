#pragma once
// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
* Structure holding the parameters of creating the google charts
*/
struct FSLGoogleChartsParameters
{
	// Should include legend at the bottom
	uint8 bLegend : 1;

	// Use tooltips
	uint8 bTooltips : 1;

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
		const FString& InLogDir,
		const FString& InEpId,
		const FSLGoogleChartsParameters& Params = FSLGoogleChartsParameters())
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

		// Add event times
		for (const auto& Ev : InEvents)
		{
			TimelineStr.Append(
				"\t\t [ \'" + Ev->Context() + "\' , \'" + Ev->Id + "\' , " );
			if (Params.bTooltips)
			{
				TimelineStr.Append(
					"createTooltipHTMLContent("
					+ FString::SanitizeFloat(Ev->Start) + ", " 
					+ FString::SanitizeFloat(Ev->End) + ", "
					+ Ev->Tooltip() + "), " );
			}
			TimelineStr.Append(FString::SanitizeFloat(Ev->Start * 1000.f) + " , " 
				+ FString::SanitizeFloat(Ev->End * 1000.f) + " ],\n");  // google charts needs millisecods
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
				"\t\t\t '</center>'"
				"\t }");
		}

		TimelineStr.Append(			
			"</script>\n"
			"<div id=\"event_tl\" style=\"height:900px;\"></div>");

		if (Params.bLegend)
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
