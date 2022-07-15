package com.osi.gnats.rest;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;

import com.osi.gnats.engine.CEngine;
import com.osi.gnats.server.ServerClass;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestBody;

import org.springframework.boot.autoconfigure.EnableAutoConfiguration;

@RestController
@EnableAutoConfiguration
public class MainController {
    private CEngine cEngine;
    
	// Get type of REST call (No input parameter to pass)
    @RequestMapping("/abc") // The URL path
    String get_abc() { // The method name can be freely named
System.out.println("MainController --> get_abc() starting");

		cEngine = ServerClass.getCEngine();
		if (cEngine != null) {
			cEngine.info(); // Test info function as a sample
		}

		String retString = "";
		
		retString = "{\"name\": \"1 2 3 4 5\"}"; // Return string is formatted as a JSON type

		return retString;
    }
    
    // Post type of REST call
    @RequestMapping(value = "/xyz", method = RequestMethod.POST)
    public ResponseEntity<Object> post_XYZ(@RequestBody String input) {
System.out.println("MainController --> post_XYZ() starting --> input = " + input);
       
       //return new ResponseEntity<Object>("Product is created successfully", HttpStatus.CREATED);
		return new ResponseEntity<Object>("{\"result\": \"p l m\"}", HttpStatus.CREATED);
    }
    
}