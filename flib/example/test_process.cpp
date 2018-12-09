#include "FProcess.hpp"
#include <iostream>

using namespace FStd;
#if 0
int testprocess() { return 0; }
#else
int testprocess() {
#if !defined(_WIN32) || defined(MSYS_PROCESS_USE_SH)
  //The following examples are for Unix-like systems and Windows through MSYS2


  std::cout << "Example 1a - the mandatory Hello World through an executable" << std::endl;
  FProcess process1a("echo Hello World", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  });
  auto exit_status = process1a.get_exit_status();
  std::cout << "Example 1a process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
#if !defined(_WIN32) && !defined(_F_USE_MEMTRACK)
  std::cout << std::endl << "Example 1b - Hello World through a function on Unix-like systems" << std::endl;
  FProcess process1b([] {
    std::cout << "Hello World" << std::endl;
    exit(0);
  }, [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  });
  exit_status = process1b.get_exit_status();
  std::cout << "Example 1b process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
#endif
  
  
  std::cout << std::endl << "Example 2 - cd into a nonexistent directory" << std::endl;
  FProcess process2("cd a_nonexistent_directory", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  }, [](const char *bytes, size_t n) {
    std::cout << "Output from stderr: " << std::string(bytes, n);
    //add a newline for prettier output on some platforms:
    if(bytes[n-1]!='\n')
      std::cout << std::endl;
  });
  exit_status = process2.get_exit_status();
  std::cout << "Example 2 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 3 - async sleep process" << std::endl;
  std::thread thread3([]() {
    FProcess process3("sleep 5");
    auto exit_status = process3.get_exit_status();
    std::cout << "Example 3 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  });
  thread3.detach();
  std::this_thread::sleep_for(std::chrono::seconds(10));
  
  
  std::cout << std::endl << "Example 4 - killing async sleep process after 5 seconds" << std::endl;
  auto process4 = std::make_shared<FProcess>("sleep 10");
  std::thread thread4([process4]() {
    auto exit_status = process4->get_exit_status();
    std::cout << "Example 4 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  });
  thread4.detach();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  process4->kill();
  std::this_thread::sleep_for(std::chrono::seconds(5));


  std::cout << std::endl << "Example 5 - multiple commands, stdout and stderr" << std::endl;
  FProcess process5("echo Hello && ls an_incorrect_path", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  }, [](const char *bytes, size_t n) {
    std::cout << "Output from stderr: " << std::string(bytes, n);
    //add a newline for prettier output on some platforms:
    if(bytes[n-1]!='\n')
      std::cout << std::endl;
  });
  exit_status = process5.get_exit_status();
  std::cout << "Example 5 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 6 - run bash with input from stdin" << std::endl;
  FProcess process6("bash", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  }, nullptr, true);
  process6.write("echo Hello from bash\n");
  process6.write("exit\n");
  exit_status = process6.get_exit_status();
  std::cout << "Example 6 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 7 - send data to cat through stdin" << std::endl;
  FProcess process7("cat", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  }, nullptr, true);
  process7.write("Hello cat\n");
  process7.close_stdin();
  exit_status = process7.get_exit_status();
  std::cout << "Example 7 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));


  std::cout << std::endl << "Example 8 - demonstrates Process::try_get_exit_status" << std::endl;
  FProcess process8("sleep 5");
  while(!process8.try_get_exit_status(exit_status)) {
    std::cout << "Example 8 process is running" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
  std::cout << "Example 8 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;


#else
  //Examples for Windows without MSYS2


  std::cout << "Example 1 - the mandatory Hello World" << std::endl;
  FProcess process1("cmd /C echo Hello World", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  });
  auto exit_status = process1.get_exit_status();
  std::cout << "Example 1 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 2 - cd into a nonexistent directory" << std::endl;
  FProcess process2("cmd /C cd a_nonexistent_directory", "", [](const char *bytes, size_t n) {
    std::cout << "Output from stdout: " << std::string(bytes, n);
  }, [](const char *bytes, size_t n) {
    std::cout << "Output from stderr: " << std::string(bytes, n);
    //add a newline for prettier output on some platforms:
    if(bytes[n-1]!='\n')
      std::cout << std::endl;
  });
  exit_status = process2.get_exit_status();
  std::cout << "Example 2 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 3 - async sleep process" << std::endl;
  std::thread thread3([]() {
    FProcess process3("timeout 5");
    auto exit_status=process3.get_exit_status();
    std::cout << "Example 3 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  });
  thread3.detach();
  std::this_thread::sleep_for(std::chrono::seconds(10));
  
  
  std::cout << std::endl << "Example 4 - killing async sleep process after 5 seconds" << std::endl;
  auto process4 = std::make_shared<FProcess>("timeout 10");
  std::thread thread4([process4]() {
    auto exit_status=process4->get_exit_status();
    std::cout << "Example 4 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;
  });
  thread4.detach();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  process4->kill();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  
  std::cout << std::endl << "Example 5 - demonstrates Process::try_get_exit_status" << std::endl;
  FProcess process5("timeout 5");
  while(!process5.try_get_exit_status(exit_status)) {
    std::cout << "Example 5 process is running" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
  std::cout << "Example 5 process returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << std::endl;


#endif

  return 0;
}

#endif

int main()
{
  testprocess();
  return 0;
}