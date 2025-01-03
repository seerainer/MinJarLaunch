use std::env;
use std::ffi::OsString;
use std::process::{Command, exit};
use std::ptr::null_mut;

#[cfg(windows)]
use std::os::windows::ffi::OsStringExt;
#[cfg(windows)]
use winapi::um::shellapi::CommandLineToArgvW;
#[cfg(windows)]
use winapi::um::winuser::{MessageBoxW, MB_ICONERROR, MB_ICONEXCLAMATION, MB_OK};
#[cfg(windows)]
use winapi::um::processthreadsapi::{CreateProcessW, PROCESS_INFORMATION, STARTUPINFOW};
#[cfg(windows)]
use winapi::um::handleapi::CloseHandle;
#[cfg(windows)]
use winapi::um::synchapi::WaitForSingleObject;
#[cfg(windows)]
use winapi::um::winbase::INFINITE;

#[cfg(windows)]
fn show_error_and_exit(message: &str) {
    let wide_message: Vec<u16> = OsString::from(message).encode_wide().chain(Some(0)).collect();
    unsafe {
        MessageBoxW(null_mut(), wide_message.as_ptr(), wide_message.as_ptr(), MB_ICONERROR | MB_OK);
    }
    exit(1);
}

#[cfg(unix)]
fn show_error_and_exit(message: &str) {
    eprintln!("{}", message);
    exit(1);
}

fn is_java_in_path() -> bool {
    if cfg!(windows) {
        let output = Command::new("cmd").args(&["/C", "where java"]).output();
        output.map_or(false, |o| o.status.success())
    } else {
        let output = Command::new("sh").arg("-c").arg("which java").output();
        output.map_or(false, |o| o.status.success())
    }
}

#[cfg(windows)]
fn main() {
    if is_java_in_path() {
        let cmd_line = unsafe { CommandLineToArgvW(winapi::um::processenv::GetCommandLineW(), &mut 0) };
        if cmd_line.is_null() {
            show_error_and_exit("Failed to get command line arguments");
        }

        let args: Vec<OsString> = unsafe {
            let mut args = Vec::new();
            let mut i = 0;
            while !(*cmd_line.offset(i)).is_null() {
                args.push(OsString::from_wide(std::slice::from_raw_parts(*cmd_line.offset(i), 1)));
                i += 1;
            }
            args
        };

        if args.len() > 1 {
            let mut command = OsString::from("javaw -jar ");
            command.push(&args[1]);

            for arg in &args[2..] {
                command.push(" ");
                command.push(arg);
            }

            let mut si: STARTUPINFOW = unsafe { std::mem::zeroed() };
            si.cb = std::mem::size_of::<STARTUPINFOW>() as u32;
            let mut pi: PROCESS_INFORMATION = unsafe { std::mem::zeroed() };

            let success = unsafe {
                CreateProcessW(
                    null_mut(),
                    command.encode_wide().chain(Some(0)).collect::<Vec<u16>>().as_mut_ptr(),
                    null_mut(),
                    null_mut(),
                    false as i32,
                    0,
                    null_mut(),
                    null_mut(),
                    &mut si,
                    &mut pi,
                )
            };

            if success == 0 {
                show_error_and_exit("javaw.exe not found!");
            }

            unsafe {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        } else {
            show_error_and_exit("No file specified!");
        }
    } else {
        show_error_and_exit("JAVA not found!\n\nDownload and install a JRE.");
    }
}

#[cfg(unix)]
fn main() {
    if is_java_in_path() {
        let args: Vec<String> = env::args().collect();
        if args.len() > 1 {
            let mut java_cmd = vec!["java".to_string(), "-jar".to_string()];
            java_cmd.extend_from_slice(&args[1..]);

            let status = Command::new("java").args(&java_cmd).status();
            if let Err(e) = status {
                eprintln!("Error: java not found or error executing command: {}", e);
                exit(1);
            }
        } else {
            eprintln!("No file specified!");
            exit(1);
        }
    } else {
        eprintln!("JAVA not found!\n\nDownload and install a JRE.");
        exit(1);
    }
}
