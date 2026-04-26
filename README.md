\documentclass[12pt]{article}

\usepackage[a4paper, margin=1in]{geometry}
\usepackage{titlesec}
\usepackage{hyperref}
\usepackage{longtable}
\usepackage{setspace}

\titleformat{\section}{\large\bfseries}{\thesection}{1em}{}
\titleformat{\subsection}{\normalsize\bfseries}{\thesubsection}{1em}{}

\title{\textbf{FreshSense Pro: AIoT-Based Food Spoilage Prediction System}}
\author{}
\date{}

\begin{document}

\maketitle
\onehalfspacing

\section{Overview}

FreshSense Pro is an AI-enabled IoT system designed to monitor and predict food spoilage using environmental and gas sensor data. The system collects real-time data from sensors, processes it on an ESP32 microcontroller, and uploads it to ThingSpeak Cloud. This data is then used to train machine learning models for predictive analysis.

The project integrates embedded systems, cloud computing, and machine learning into a unified pipeline.

\section{System Architecture}

\textbf{Sensors → ESP32 (Processing and Calibration) → ThingSpeak Cloud → Machine Learning Model}

\subsection{Data Flow}
\begin{enumerate}
    \item Sensors capture temperature, humidity, and gas concentration.
    \item ESP32 processes and calibrates the raw data.
    \item Data is transmitted to ThingSpeak at regular intervals.
    \item Cloud data is used to train machine learning models.
\end{enumerate}

\section{Hardware Components}

\begin{itemize}
    \item ESP32-S3 (Seeed XIAO)
    \item MQ-135 Gas Sensor
    \item DHT11 Temperature and Humidity Sensor
    \item LED Indicator
\end{itemize}

\section{Software Stack}

\begin{itemize}
    \item Embedded C++ (Arduino Framework)
    \item ESPAsyncWebServer
    \item ArduinoJson
    \item ThingSpeak API
    \item Python (for machine learning model training)
\end{itemize}

\section{Core Functional Logic}

\subsection{Sensor Data Acquisition}

The system continuously reads:
\begin{itemize}
    \item Temperature (°C)
    \item Humidity (\%)
    \item Gas concentration (VOC levels)
\end{itemize}

\subsection{Gas Sensor Calibration}

\subsubsection{Resistance Calculation}
\begin{equation}
Rs = RL \times \left(\frac{4095}{ADC_{raw}} - 1\right)
\end{equation}

\subsubsection{Environmental Compensation}
\begin{equation}
Correction = -0.015 (T - 20) - 0.005 (H - 33) + 1
\end{equation}

\subsubsection{Gas Concentration Proxy}
\begin{equation}
PPM_{proxy} = \left(\frac{R0}{Rs_{compensated}}\right) \times 1000
\end{equation}

\subsection{Baseline Calibration (R0)}

\begin{itemize}
    \item The system collects 50 samples during initialization.
    \item Computes baseline resistance under clean air conditions.
    \item Ensures stable long-term measurements.
\end{itemize}

\section{ThingSpeak Cloud Integration}

The ESP32 uploads data every 20 seconds:

\begin{itemize}
    \item Field 1: Temperature
    \item Field 2: Humidity
    \item Field 3: Gas Deviation
\end{itemize}

\subsection{Example Code}
\begin{verbatim}
ThingSpeak.setField(1, t);
ThingSpeak.setField(2, h);
ThingSpeak.setField(3, deviation);

ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
\end{verbatim}

\section{Dataset Generation}

The transmitted data forms a structured dataset:

\begin{longtable}{|c|c|c|}
\hline
Temperature & Humidity & Gas Deviation \\
\hline
33.3 & 40.1 & 6.81 \\
33.8 & 40.6 & 18.78 \\
\hline
\end{longtable}

This dataset is used for training machine learning models.

\section{Machine Learning Pipeline}

\subsection{Model}
Random Forest Classifier

\subsection{Workflow}
\begin{enumerate}
    \item Export data from ThingSpeak
    \item Label data (Fresh, Spoiling, Spoiled)
    \item Train the model using Python
    \item Evaluate performance
    \item Use model for prediction
\end{enumerate}

\subsection{Advantages}
\begin{itemize}
    \item Handles non-linear relationships effectively
    \item Robust to noise in sensor data
    \item Provides feature importance insights
\end{itemize}

\section{Real-Time System Features}

\subsection{Non-Blocking Architecture}

\begin{itemize}
    \item Uses \texttt{millis()} instead of \texttt{delay()}
    \item Enables concurrent execution of:
    \begin{itemize}
        \item Sensor readings
        \item Web server operations
        \item Cloud communication
    \end{itemize}
\end{itemize}

\subsection{LED Status Indication}

\begin{itemize}
    \item OFF: Fresh condition
    \item Slow Blink: Spoiling detected
    \item Fast Blink: Critical condition
\end{itemize}

\section{Embedded Web Dashboard}

\begin{itemize}
    \item Built using Tailwind CSS and JavaScript
    \item Hosted directly on ESP32
    \item Displays:
    \begin{itemize}
        \item Temperature
        \item Humidity
        \item Gas levels
        \item Freshness score
    \end{itemize}
\end{itemize}

\section{Setup and Installation}

\subsection{Hardware Setup}
\begin{itemize}
    \item Connect sensors to ESP32
    \item Ensure MQ-135 undergoes 24-hour burn-in
\end{itemize}

\subsection{Arduino IDE Setup}

Install:
\begin{itemize}
    \item ESPAsyncWebServer
    \item AsyncTCP
    \item DHT sensor library
    \item ArduinoJson
    \item ThingSpeak
\end{itemize}

\subsection{Configuration}

\begin{verbatim}
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

unsigned long myChannelNumber = YOUR_CHANNEL_ID;
const char * myWriteAPIKey = "YOUR_API_KEY";
\end{verbatim}

\subsection{Upload Instructions}

\begin{itemize}
    \item Select board: Seeed XIAO ESP32S3
    \item Enable USB CDC On Boot
    \item Upload code
    \item Open Serial Monitor at 115200 baud
\end{itemize}

\section{Future Enhancements}

\begin{itemize}
    \item Deploy ML model on ESP32 (edge inference)
    \item Add mobile/web monitoring interface
    \item Integrate advanced gas sensors
    \item Implement alert systems (SMS/Email)
\end{itemize}

\section{Conclusion}

FreshSense Pro demonstrates a complete AIoT pipeline combining sensor data acquisition, cloud integration, and machine learning. The system enables real-time monitoring and predictive analysis of food spoilage, providing a scalable solution for smart food safety applications.

\end{document}
