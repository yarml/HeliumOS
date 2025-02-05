use core::{marker::PhantomData, mem};

pub trait True {}
pub trait False {}

pub struct Assert<const CONDITION: bool>;

impl True for Assert<true> {}
impl False for Assert<false> {}

pub struct SameSize<T, Q> {
  _phantom: PhantomData<(T, Q)>,
}

impl<T, Q> True for SameSize<T, Q> where
  Assert<{ mem::size_of::<T>() == mem::size_of::<Q>() }>: True
{
}
impl<T, Q> False for SameSize<T, Q> where
  Assert<{ mem::size_of::<T>() == mem::size_of::<Q>() }>: False
{
}
