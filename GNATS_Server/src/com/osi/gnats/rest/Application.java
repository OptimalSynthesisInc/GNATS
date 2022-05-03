package com.osi.gnats.rest;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.context.annotation.Configuration;

import com.osi.gnats.engine.CEngine;

import org.springframework.boot.context.web.SpringBootServletInitializer;

import java.util.Properties;

@Configuration
@SpringBootApplication
@EnableAutoConfiguration
public class Application extends SpringBootServletInitializer {
	private static boolean flagInit = false;
	
	//private static CEngine cEngine;
	
	public Application() {
		init();
	}
	
	public Application(CEngine cEngine) {
		//this.cEngine = cEngine;
		
		init();
	}
	
	private void init() {
		if (!flagInit) {
			flagInit = true;

			Properties properties = new Properties();
			properties.setProperty("spring.main.banner-mode", "off");
			properties.setProperty("logging.pattern.console", "");
			
			SpringApplication application = new SpringApplication(Application.class);
			application.setDefaultProperties(properties);
			application.run(new String[] {});
			
			System.out.println("  GNATS REST server ready");
		}
	}
	
}
