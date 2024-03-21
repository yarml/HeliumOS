pub unsafe fn unchecked_cast<T, U>(r: &T) -> &U {
  unsafe { (r as *const T as *const U).as_ref() }.unwrap()
}
