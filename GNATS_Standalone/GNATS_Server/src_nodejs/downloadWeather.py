'''
downloadWeather.py

Author: Oliver Chen
Company: Optimal Synthesis Inc.
Update: 05.11.2018

This program connect to aviationweather.gov website, get the page content, extract required weather info from the page content and finally save it to files.
'''

import scrapy
from scrapy.selector import Selector
import time
import os

class WeatherSpider(scrapy.Spider):
    name = 'weatherSpider'
    
    # List all URL to be visited
    start_urls = [
        'https://www.aviationweather.gov/sigmet/data?hazard=all&loc=all',
        'https://www.aviationweather.gov/metar/data?ids=&format=raw&date=0&hours=0',
        'https://www.aviationweather.gov/airep/data?id=&distance=200&format=raw&type=&age=15&layout=on&date='
    ]
 
    def parse(self, response):
        NATS_SERVER_HOME = os.environ['NATS_SERVER_HOME']
        WEATHER_DIR_PATH = NATS_SERVER_HOME + "/share/tg/weather"

        page = response.url.split("/")[-2]
        
        htmlDoc = response.body

        # We check the URL and distinguish the work flow
        # Different URL page contains different info.  The page formats are different.  We have to handle them individually.

        # If URL contains "sigmet"
        if ('sigmet' in response.url) :
            filename_SIGMET = time.strftime('%Y%m%d_%H%M%S', time.localtime()) + '.sigmet'
            
            file_SIGMET = open(WEATHER_DIR_PATH + "/" + filename_SIGMET, 'wb')
            
            title = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/div[@id="title"]/text()').extract_first()
            if (len(title.strip()) > 0) :
                file_SIGMET.write(title + "\n\n")
                
            array_children = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/*')
            for i in range(0, len(array_children)) :
                node_layer_1 = array_children[i]
                if ('p' == node_layer_1.xpath('name()').extract()[0]) :
                    node_layer_2 = node_layer_1.xpath('//b')
                    if not (node_layer_2 is None) :
                        file_SIGMET.write(node_layer_2.xpath('text()').extract()[0] + '\n')
                elif ('b' == node_layer_1.xpath('name()').extract()[0]) :
                    file_SIGMET.write(node_layer_1.xpath('text()').extract()[0] + '\n')
                elif ('pre' == node_layer_1.xpath('name()').extract()[0]) :
                    file_SIGMET.write(node_layer_1.xpath('text()').extract()[0] + '\n\n')
                    
            file_SIGMET.close()
        elif ('metar' in response.url) : # If URL contains "metar"
            filename_METAR = time.strftime('%Y%m%d_%H%M%S', time.localtime()) + '.metar'
            
            file_METAR = open(WEATHER_DIR_PATH + "/" + filename_METAR, 'wb')
            
            title = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/div[@id="title"]/text()').extract_first()
            if (len(title.strip()) > 0) :
                file_METAR.write(title + "\n\n")

            array_children = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/*')
            for i in range(0, len(array_children)) :
                node_layer_1 = array_children[i]

                if ('p' == node_layer_1.xpath('name()').extract()[0]) :
                    node_layer_2 = node_layer_1.xpath('//b')
                    if not (node_layer_2 is None) :
                        file_METAR.write(node_layer_2.xpath('text()').extract()[0] + '\n')
                elif ('b' == node_layer_1.xpath('name()').extract()[0]) :
                    file_METAR.write(node_layer_1.xpath('text()').extract()[0] + '\n')

            array_data = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/text()')
            for i in range(0, len(array_data)) :
                if (len(array_data[i].extract().strip()) > 0) :
                    file_METAR.write("\n" + array_data[i].extract())
            
            file_METAR.close()
        elif ('airep' in response.url) : # If URL contains "airep"
            filename_AIREP = time.strftime('%Y%m%d_%H%M%S', time.localtime()) + '.airep'
            
            file_AIREP = open(WEATHER_DIR_PATH + "/" + filename_AIREP, 'wb')
            
            title = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/div[@id="title"]/text()').extract_first()
            if (len(title.strip()) > 0) :
                file_AIREP.write(title + "\n\n")
            
            array_children = Selector(text=htmlDoc).xpath('//div[@id="awc_main_content"]/div/*')
            for i in range(0, len(array_children)) :
                node_layer_1 = array_children[i]
                
                if ('p' == node_layer_1.xpath('name()').extract()[0]) :
                    node_layer_2 = node_layer_1.xpath('//b')
                    if not (node_layer_2 is None) :
                        file_AIREP.write(node_layer_2.xpath('text()').extract()[0] + '\n\n')
                elif ('code' == node_layer_1.xpath('name()').extract()[0]) :
                    file_AIREP.write(node_layer_1.xpath('text()').extract()[0] + '\n')
            
            file_AIREP.close()
