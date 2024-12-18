#include <WiFi.h>
#include "settings.h"

extern int score;
extern char currentPlayerName[11]; // Declare the external player name variable


extern int lives;

const char* ssid     = "Martin Router King"; // Ersetzen Sie dies durch Ihre SSID
const char* passphrase = "25261100585769915716"; // Ersetzen Sie dies durch Ihr Passwort

WiFiServer server(80);

char linebuf[80];
int charcount=0;

enum PAGE_TO_SEND
{
  Stats,
  Metrics
};

void wifi_client_setup() {
    WiFi.begin(ssid, passphrase);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}


void sendHTTPHeader(WiFiClient client, const char* contentType) {
	client.println("HTTP/1.1 200 OK");
	client.print("Content-type:");
	client.println(contentType);
	client.println();
}

void sendHTMLPage(WiFiClient client, const String& content) {
	sendHTTPHeader(client, "text/html");
	client.println("<html>");
	client.println("<head>");
	client.println("<style>");
	client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 0; }");
	client.println("h1, h2 { background-color: #4CAF50; color: white; padding: 10px; }");
	client.println("ul { list-style-type: none; padding: 0; }");
	client.println("li { padding: 8px; margin-bottom: 7px; background-color: #f9f9f9; border: 1px solid #ddd; }");
	client.println("table { width: 100%; border-collapse: collapse; }");
	client.println("td { padding: 8px; border: 1px solid #ddd; }");
	client.println("button, input[type='submit'] { background-color: #4CAF50; color: white; border: none; padding: 10px 20px; cursor: pointer; }");
	client.println("button:hover, input[type='submit']:hover { background-color: #45a049; }");
	client.println("@media (max-width: 600px) { h1, h2 { font-size: 18px; } }");
	client.println("</style>");
	client.println("<script>");
	client.println("function refreshPage() { location.reload(); }");
	client.println("function fetchMetrics() {");
	client.println("  fetch('/metrics').then(response => response.text()).then(data => {");
	client.println("    document.getElementById('metrics').innerText = data;");
	client.println("  });");
	client.println("}");
	client.println("window.onload = fetchMetrics;");
	client.println("</script>");
	client.println("</head>");
	client.println("<body>");
	client.print(content);
	client.println("<h2>Metrics</h2><pre id='metrics'>Loading...</pre>");
	client.println("</body>");
	client.println("</html>");
}

String generateStatsPage() {
	String page;
	page += "<h1>TWANG32 Play Stats</h1><ul>";
	page += "<li>Games played: " + String(user_settings.games_played) + "</li>";
	if (user_settings.games_played > 0) {
		page += "<li>Average score: " + String(user_settings.total_points / user_settings.games_played) + "</li>";
	}
	page += "<li>High score: " + String(user_settings.high_score) + "</li>";
	page += "<li>Boss kills: " + String(user_settings.boss_kills) + "</li>";
	page += "</ul><button onClick=\"refreshPage()\">Refresh</button>";
	page += "<h2>Leaderboard</h2><ul>";
	for (int i = 0; i < 5; i++) {
		page += "<li>Rank " + String(i + 1) + ": " + String(user_settings.leaderboard[i].name) + " - " + String(user_settings.leaderboard[i].score) + " points</li>";
		Serial.print("Leaderboard Display Name: ");
		Serial.println(user_settings.leaderboard[i].name);
	}
	page += "</ul>";
	page += "<h2>Enter Player Name</h2><form><input type='text' name='N' maxlength='10' value='" + String(user_settings.leaderboard[0].name) + "'><input type='submit'></form>";
	page += "<h2>Adjustable Settings </h2><table>";
	page += "<tr><td>LED Count (60-" + String(MAX_LEDS) + ")</td><td><form><input type='number' name='C' value='" + String(user_settings.led_count) + "' min='60' max='" + String(MAX_LEDS) + "'><input type='submit'></form></td></tr>";
	page += "<tr><td>Brightness (10-255)</td><td><form><input type='number' name='B' value='" + String(user_settings.led_brightness) + "' min='10' max='255'><input type='submit'></form></td></tr>";
	page += "<tr><td>Sound Volume (0-255)</td><td><form><input type='number' name='S' value='" + String(user_settings.audio_volume) + "' min='0' max='255'><input type='submit'></form></td></tr>";
	page += "<tr><td>Joystick Deadzone (3-12)</td><td><form><input type='number' name='D' value='" + String(user_settings.joystick_deadzone) + "' min='3' max='12'><input type='submit'></form></td></tr>";
	page += "<tr><td>Attack Sensitivity (20000-35000)</td><td><form><input type='number' name='A' value='" + String(user_settings.attack_threshold) + "' min='2000' max='35000'><input type='submit'></form></td></tr>";
	page += "<tr><td>Lives Per Game</td><td>" + String(lives) + "</td></tr>";
	page += "<tr><td>Game Difficulty (1=Easy, 2=Medium, 3=Hard)</td><td><form><input type='number' name='G' value='" + String(user_settings.difficulty) + "' min='1' max='3'><input type='submit'></form></td></tr>";
	page += "</table>";
#define ENABLE_PROMETHEUS_METRICS_ENDPOINT
#ifdef ENABLE_PROMETHEUS_METRICS_ENDPOINT
	page += "<ul><li><a href=\"/metrics\">Metrics</a></li></ul>";
#endif
	return page;
}

void sendStatsPage(WiFiClient client) {
	String pageContent = generateStatsPage();
	sendHTMLPage(client, pageContent);
}

#ifdef ENABLE_PROMETHEUS_METRICS_ENDPOINT
// We need to use print() here since println() prints newlines as CR/LF, which
// Prometheus cannot handle.
#define __prom_metric(metric_name, metric_description, value) \
	client.print("# HELP " metric_name " " metric_description "\n");   \
	client.print("# TYPE " metric_name " gauge\n");                    \
	client.print(metric_name " ");                                     \
	client.print(value);                                               \
	client.print("\n");

static void sendMetricsPage(WiFiClient client)
{
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: text/plain; charset=utf-8");
	client.println("Server: twang_exporter");
	client.println();
	__prom_metric("twang_games_played", "Number of games played", user_settings.games_played);
	__prom_metric("twang_total_points", "Total points", user_settings.total_points);
	__prom_metric("twang_high_score", "High score", user_settings.high_score);
	__prom_metric("twang_boss_kills", "Boss kills", user_settings.boss_kills);
	// Add leaderboard metrics
	for (int i = 0; i < 5; i++) {
		client.print("twang_leaderboard_score{rank=\"");
		client.print(i + 1);
		client.print("\",name=\"");
		client.print(user_settings.leaderboard[i].name);
		client.print("\"} ");
		client.print(user_settings.leaderboard[i].score);
		client.print("\n");
	}
}

#undef __prom_metric
#endif // ENABLE_PROMETHEUS_METRICS_ENDPOINT


void ap_client_check() {
	WiFiClient client = server.available();   // listen for incoming clients

	if (!client) {
		return;
	}

	bool currentLineIsBlank = true;
	PAGE_TO_SEND page_to_send = Stats;

	while (client.connected()) {
		if (client.available()) {
			char c = client.read();
			linebuf[charcount] = c;
			if (charcount < sizeof(linebuf) - 1) {
				charcount++;
			}

			if (c == '\n' && currentLineIsBlank) {
				switch (page_to_send) {
				case Stats:
					sendStatsPage(client);
					break;
#ifdef ENABLE_PROMETHEUS_METRICS_ENDPOINT
				case Metrics:
					sendMetricsPage(client);
					break;
#endif
				}
				break;
			}

			if (c == '\n') {
				currentLineIsBlank = true;

				if (strstr(linebuf, "GET /?") > 0) {
					String line = String(linebuf);
					int start = line.indexOf('=', 0);
					if (start != -1) {
						char paramCode = line.charAt(start - 1);
						int finish = line.indexOf('H', start + 1) - 1;
						if (finish != -1) {
							String val = line.substring(start + 1, finish);
							if (paramCode == 'N') { // 'N' for playerName
								String decodedName = "";
								for (int i = 0; i < val.length(); i++) {
									if (val[i] == '%') {
										int code;
										sscanf(val.substring(i + 1, i + 3).c_str(), "%x", &code);
										decodedName += char(code);
										i += 2;
									} else if (val[i] == '+') {
										decodedName += ' ';
									} else {
										decodedName += val[i];
									}
								}
								strncpy(currentPlayerName, decodedName.c_str(), sizeof(currentPlayerName) - 1);
								currentPlayerName[sizeof(currentPlayerName) - 1] = '\0';
								Serial.print("Player Name Set To: ");
								Serial.println(currentPlayerName);
							} else {
								change_setting(paramCode, val.toInt());
							}
						}
					}
					page_to_send = Stats;
				}
#ifdef ENABLE_PROMETHEUS_METRICS_ENDPOINT
				else if (strstr(linebuf, "GET /metrics")) {
					page_to_send = Metrics;
				}
#endif
				currentLineIsBlank = true;
				memset(linebuf, 0, sizeof(linebuf));
				charcount = 0;
			} else if (c != '\r') {
				currentLineIsBlank = false;
			}
		}
	}
}
