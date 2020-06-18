//
// Created by Денис Попеску on 13/06/2020.
//
#include <set>
#include <string>
#include "../include/ILogger.h"

namespace {
    class LoggerImpl : public ILogger {
    private:
        const std::string fileNameLog = "log.txt";
        std::set<void *> clients;
        FILE *logStream;
    public:
        LoggerImpl() {
            logStream = fopen(fileNameLog.c_str(), "w");
            if (logStream == nullptr) {
                fprintf(stderr, "Couldn't create log file\n");
            }
        }

        void destroyLogger(void *pClient) override {
            auto iter = clients.find(pClient);
            if (iter != clients.end()) {
                clients.erase(iter);
            }

            if (clients.empty()) {
                fclose(logStream);
                logStream = nullptr;
            }
        }

        void log(const char *pMsg, RESULT_CODE err) override {
            const char *startMsg;
            switch (err) {
                case RESULT_CODE::SUCCESS:
                    startMsg = "INFO: ";
                    break;
                case RESULT_CODE::OUT_OF_MEMORY:
                    startMsg = "ERROR (out of memory): ";
                    break;
                case RESULT_CODE::BAD_REFERENCE:
                    startMsg = "ERROR (bad reference): ";
                    break;
                case RESULT_CODE::WRONG_DIM:
                    startMsg = "ERROR (wrong dimension): ";
                    break;
                case RESULT_CODE::DIVISION_BY_ZERO:
                    startMsg = "ERROR (division by zero): ";
                    break;
                case RESULT_CODE::NAN_VALUE:
                    startMsg = "ERROR (not a number): ";
                    break;
                case RESULT_CODE::FILE_ERROR:
                    startMsg = "ERROR (file error): ";
                    break;
                case RESULT_CODE::OUT_OF_BOUNDS:
                    startMsg = "ERROR (out of bonds): ";
                    break;
                case RESULT_CODE::NOT_FOUND:
                    startMsg = "ERROR (not found): ";
                    break;
                case RESULT_CODE::WRONG_ARGUMENT:
                    startMsg = "ERROR (wrong argument): ";
                    break;
                case RESULT_CODE::CALCULATION_ERROR:
                    startMsg = "ERROR (calculation error): ";
                    break;
                case RESULT_CODE::MULTIPLE_DEFINITION:
                    startMsg = "ERROR (multiple definition): ";
                    break;
            }
            fprintf(logStream, "%s %s\n", startMsg, pMsg);
        }

        RESULT_CODE setLogFile(const char *pLogFile) override {
            fclose(logStream);
            logStream = fopen(pLogFile, "w");
            if (logStream != nullptr) {
                return RESULT_CODE::SUCCESS;
            }
            return RESULT_CODE::FILE_ERROR;
        }

        static LoggerImpl * getInstance() {
            static LoggerImpl logger;
            return &logger;
        }

        void addClient(void *pCLient) {
            clients.insert(pCLient);
        }

    protected:
        ~LoggerImpl() override {
            if (logStream != nullptr) {
                fclose(logStream);
                logStream = nullptr;
            }
        }
    };
}


ILogger * ILogger::createLogger( void* pClient ) {
    if (pClient == nullptr) {
        return nullptr;
    }

    auto logger = LoggerImpl::getInstance();

    if (logger == nullptr) {
        fprintf(stderr, "No memory for logger\n");
        return nullptr;
    }

    logger->addClient(pClient);
    return logger;
}



