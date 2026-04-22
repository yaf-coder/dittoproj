export default function Input({ label, className = '', ...props }) {
  return (
    <div className="flex flex-col gap-1.5">
      {label && (
        <label className="text-[13px] font-medium text-apple-sub px-1">
          {label}
        </label>
      )}
      <input
        className={[
          'w-full bg-apple-gray rounded-2xl',
          'px-4 py-3.5 text-[17px] text-apple-text',
          'placeholder:text-apple-sub/60',
          'outline-none transition',
          'focus:ring-2 focus:ring-apple-blue/40',
          className,
        ].join(' ')}
        {...props}
      />
    </div>
  )
}
