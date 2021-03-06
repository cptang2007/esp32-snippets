/*
 * FreeRTOS.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "FreeRTOS.h"
#include <esp_log.h>
#include "sdkconfig.h"

static const char* LOG_TAG = "FreeRTOS";

/**
 * Sleep for the specified number of milliseconds.
 * @param[in] ms The period in milliseconds for which to sleep.
 */
void FreeRTOS::sleep(uint32_t ms) {
	::vTaskDelay(ms/portTICK_PERIOD_MS);
} // sleep


/**
 * Start a new task.
 * @param[in] task The function pointer to the function to be run in the task.
 * @param[in] taskName A string identifier for the task.
 * @param[in] param An optional parameter to be passed to the started task.
 * @param[in] stackSize An optional paremeter supplying the size of the stack in which to run the task.
 */
void FreeRTOS::startTask(void task(void*), std::string taskName, void *param, int stackSize) {
	::xTaskCreate(task, taskName.data(), stackSize, param, 5, NULL);
} // startTask


/**
 * Delete the task.
 * @param[in] pTask An optional handle to the task to be deleted.  If not supplied the calling task will be deleted.
 */
void FreeRTOS::deleteTask(TaskHandle_t pTask) {
	::vTaskDelete(pTask);
} // deleteTask


/**
 * Get the time in milliseconds since the %FreeRTOS scheduler started.
 * @return The time in milliseconds since the %FreeRTOS scheduler started.
 */
uint32_t FreeRTOS::getTimeSinceStart() {
	return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);
} // getTimeSinceStart

/*
 * 	public:
		Semaphore(std::string = "<Unknown>");
		~Semaphore();
		void give();
		void take(std::string owner="<Unknown>");
		void take(uint32_t timeoutMs, std::string owner="<Unknown>");
	private:
		SemaphoreHandle_t m_semaphore;
		std::string m_name;
		std::string m_owner;
	};
 *
 */

/**
 * @brief Wait for a semaphore to be released by trying to take it and
 * then releasing it again.
 * @param [in] owner A debug tag.
 * @return The value associated with the semaphore.
 */
uint32_t FreeRTOS::Semaphore::wait(std::string owner) {
	ESP_LOGV(LOG_TAG, "Semaphore waiting: %s for %s", toString().c_str(), owner.c_str());
	xSemaphoreTake(m_semaphore, portMAX_DELAY);
	m_owner = owner;
	xSemaphoreGive(m_semaphore);
	ESP_LOGV(LOG_TAG, "Semaphore released: %s", toString().c_str());
	m_owner = "<N/A>";
	return m_value;
} // wait

FreeRTOS::Semaphore::Semaphore(std::string name) {
	m_semaphore = xSemaphoreCreateMutex();
	m_name      = name;
	m_owner     = "<N/A>";
	m_value     = 0;
}

FreeRTOS::Semaphore::~Semaphore() {
	vSemaphoreDelete(m_semaphore);
}


/**
 * @brief Give a semaphore.
 * The Semaphore is given.
 */
void FreeRTOS::Semaphore::give() {
	xSemaphoreGive(m_semaphore);
	ESP_LOGV(LOG_TAG, "Semaphore giving: %s", toString().c_str());
	m_owner = "<N/A>";
} // Semaphore::give


/**
 * @brief Give a semaphore.
 * The Semaphore is given with an associated value.
 * @param [in] value The value to associate with the semaphore.
 */
void FreeRTOS::Semaphore::give(uint32_t value) {
	m_value = value;
	give();
}


/**
 * @brief Give a semaphore from an ISR.
 */
void FreeRTOS::Semaphore::giveFromISR() {
	BaseType_t higherPriorityTaskWoken;
	xSemaphoreGiveFromISR(m_semaphore, &higherPriorityTaskWoken);
} // giveFromISR


/**
 * @brief Take a semaphore.
 * Take a semaphore and wait indefinitely.
 */
void FreeRTOS::Semaphore::take(std::string owner)
{

	ESP_LOGV(LOG_TAG, "Semaphore taking: %s for %s", toString().c_str(), owner.c_str());
	xSemaphoreTake(m_semaphore, portMAX_DELAY);
	m_owner = owner;
	ESP_LOGV(LOG_TAG, "Semaphore taken:  %s", toString().c_str());
} // Semaphore::take


/**
 * @brief Take a semaphore.
 * Take a semaphore but return if we haven't obtained it in the given period of milliseconds.
 * @param [in] timeoutMs Timeout in milliseconds.
 */
void FreeRTOS::Semaphore::take(uint32_t timeoutMs, std::string owner) {
	ESP_LOGV(LOG_TAG, "Semaphore taking: %s for %s", toString().c_str(), owner.c_str());
	m_owner = owner;
	xSemaphoreTake(m_semaphore, timeoutMs/portTICK_PERIOD_MS);
	ESP_LOGV(LOG_TAG, "Semaphore taken:  %s", toString().c_str());
} // Semaphore::take

std::string FreeRTOS::Semaphore::toString() {
	std::stringstream stringStream;
	stringStream << "name: "<< m_name << " (0x" << std::hex << std::setfill('0') << (uint32_t)m_semaphore << "), owner: " << m_owner;
	return stringStream.str();
}

void FreeRTOS::Semaphore::setName(std::string name) {
	m_name = name;
}

