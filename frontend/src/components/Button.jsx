import { motion } from 'framer-motion'

const variants = {
  primary:   'bg-apple-blue text-white hover:bg-apple-blue-dk',
  secondary: 'bg-apple-gray text-apple-text hover:bg-apple-gray-2',
  ghost:     'bg-transparent text-apple-blue hover:bg-apple-gray',
  danger:    'bg-apple-red text-white hover:bg-red-600',
}

export default function Button({
  children,
  variant = 'primary',
  className = '',
  fullWidth = false,
  disabled = false,
  ...props
}) {
  return (
    <motion.button
      whileTap={{ scale: disabled ? 1 : 0.96 }}
      transition={{ type: 'spring', stiffness: 400, damping: 20 }}
      disabled={disabled}
      className={[
        'inline-flex items-center justify-center gap-2',
        'rounded-2xl px-6 py-3.5',
        'text-[15px] font-semibold',
        'transition-colors duration-150',
        'disabled:opacity-40 disabled:pointer-events-none',
        variants[variant],
        fullWidth ? 'w-full' : '',
        className,
      ].join(' ')}
      {...props}
    >
      {children}
    </motion.button>
  )
}
