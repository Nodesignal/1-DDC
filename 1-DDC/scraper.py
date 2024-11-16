import requests
from bs4 import BeautifulSoup
import schedule
import time

def scrape_data():
    # URL der HTML-Seite
    url = 'http://192.168.4.1'
    
    # Sende eine HTTP-Anfrage an die URL
    response = requests.get(url)
    
    # Überprüfe, ob die Anfrage erfolgreich war
    if response.status_code == 200:
        # Parse den HTML-Inhalt
        soup = BeautifulSoup(response.content, 'html.parser')
        
        # Extrahiere die Leaderboard-Daten
        leaderboard_items = soup.select('h2:contains("Leaderboard") + ul li')
        for item in leaderboard_items:
            print(item.text)
    else:
        print(f"Fehler beim Abrufen der Seite: {response.status_code}")

# Plane die Funktion, alle 10 Minuten auszuführen
schedule.every(10).minutes.do(scrape_data)

# Endlosschleife, um den Scheduler laufen zu lassen
while True:
    schedule.run_pending()
    time.sleep(1)
