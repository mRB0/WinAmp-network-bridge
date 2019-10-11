extern crate winapi;
extern crate libloading;
extern crate encoding_rs;

use std::io::Error;
use std::ffi::{OsStr, CStr};
use std::iter::once;
use std::os::windows::ffi::OsStrExt;
use std::ptr::null_mut;
use std::os::raw::{c_int, c_char};

use winapi::um::winuser::{MB_OK, MessageBoxW};
use winapi::um::libloaderapi::{GetModuleHandleW, GetProcAddress, LoadLibraryW};

#[allow(dead_code)]
fn print_message(msg: &str) -> Result<i32, Error> {
    let wide: Vec<u16> = OsStr::new(msg).encode_wide().chain(once(0)).collect();
    let ret = unsafe {
        MessageBoxW(null_mut(), wide.as_ptr(), wide.as_ptr(), MB_OK)
    };
    if ret == 0 { Err(Error::last_os_error()) }
    else { Ok(ret) }
}

#[repr(C)]
struct InModule {
    // TODO: If plugin_ver == 0x0F000100, in_char indicates unicode (utf-16); otherwise (0x00000100) it's windows-1252

    version: c_int,
    description: *mut c_char,
    h_main_window: winapi::shared::windef::HWND,
}

#[allow(dead_code)]
fn load_library_with_libloading() {
    let vgmlib = libloading::os::windows::Library::new("C:\\Users\\mrb\\Documents\\code\\winamp-network-bridge\\plugins\\in_vgm.dll").unwrap();

    unsafe {
        
        let h_module = vgmlib.into_raw();
        let vgmlib = libloading::os::windows::Library::from_raw(h_module);
        println!("h_module = {:?}", h_module);

        let get_in_module_func: libloading::os::windows::Symbol<unsafe extern fn() -> *const InModule> = vgmlib.get(b"winampGetInModule2").unwrap();

        let plugin = get_in_module_func();
        println!("plugin_version = {:x}", (*plugin).version);

        println!("h_main_window = {:?}", (*plugin).h_main_window);

        let desc_cstr = CStr::from_ptr((*plugin).description);
        let desc_bytes = desc_cstr.to_bytes();

        let (desc, _) = encoding_rs::WINDOWS_1251.decode_without_bom_handling(desc_bytes);

        // print_message(&desc).unwrap();

        println!("{}", desc);
    };
}

// fn load_library_with_winapi() {
//     unsafe {
//         let lib_path_ptr: Vec<u16> = OsStr::new("C:\\Users\\mrb\\Documents\\code\\winamp-network-bridge\\plugins\\in_vgm.dll").encode_wide().chain(once(0)).collect();
//         let h_module = LoadLibraryW(lib_path_ptr.as_ptr());
//         println!("error: {:?}", Error::last_os_error());
//         println!("{:?}", h_module);
//     };
// }

fn main() {

    // let h_instance = GetModuleHandleW(null_mut());

    load_library_with_libloading();
    // load_library_with_winapi();

    // print_message("Hello, world!").unwrap();
}
