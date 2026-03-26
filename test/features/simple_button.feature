Feature: Simple button control
  As a firmware developer
  I want to control an button
  So that I can verify the basic GPIO functionality,
  and the application layer works correctly

  Scenario: Press button
    Given the firmware simulation is running for "XXX"
    And pin IO "15" is "1"
    And set command with pin "15" and mode "INPUT"
    And get command with pin "15"
    When I run the simulation
    Then getter response with pin "15" should be 1

  Scenario: Don't press button
    Given the firmware simulation is running for "XXX"
    And pin IO "15" is "0"
    And set command with pin "15" and mode "INPUT"
    And get command with pin "15"
    When I run the simulation
    Then getter response with pin "15" should be 0
