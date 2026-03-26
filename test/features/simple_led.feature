Feature: Simple LED control
  As a firmware developer
  I want to control an LED
  So that I can verify the basic GPIO functionality,
  and the application layer works correctly

  Scenario: Turn on an LED
    Given the firmware simulation is running for "XXX"
    And set command with pin "13" and mode "OUTPUT"
    And set command with pin "13" and value "1"
    When I run the simulation
    Then pin IO "13" should be 1

  Scenario: Turn off an LED
    Given the firmware simulation is running for "XXX"
    And set command with pin "13" and mode "OUTPUT"
    And set command with pin "13" and value "0"
    When I run the simulation
    Then pin IO "13" should be 0
