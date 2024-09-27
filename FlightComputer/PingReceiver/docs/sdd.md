\page FlightComputerPingReceiverComponent FlightComputer::PingReceiver Component
# FlightComputer::PingReceiver Component

## 1. Introduction

The `FlightComputer::PingReceiver` is a demonstration component that accepts pings from the [Health Component](../../../Svc/Health/docs/sdd.md) and has commands to disable ping responses for testing purposes.

## 2. Requirements

The requirements for `FlightComputer::PingReceiver` are as follows:

Requirement | Description | Verification Method
----------- | ----------- | -------------------
ISF-PNG-001 | The `FlightComputer::PingReceiver` component shall reply to ping requests | System test
ISF-PNG-002 | The `FlightComputer::PingReceiver` component shall provide commands to disable ping responses | System test

## 3. Design

### 3.1 Context

#### 3.1.1 Component Diagram

The `FlightComputer::PingReceiver` component has the following component diagram:

![`FlightComputer::PingReceiver` Diagram](img/PingReceiverBDD.jpg "FlightComputer::PingReceiver")

## 4. Dictionaries

## 5. Module Checklists

## 6. Unit Testing

## 7. Change Log

Date | Description
---- | -----------
4/20/2017 | Initial Version


