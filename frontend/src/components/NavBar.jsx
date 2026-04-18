import { NavLink } from 'react-router-dom'
import { motion } from 'framer-motion'
import { useLocation } from 'react-router-dom'

const TABS = [
  {
    to: '/discover',
    label: 'Discover',
    icon: (active) => (
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none">
        <circle cx="12" cy="12" r="9" stroke="currentColor" strokeWidth={active ? 2.2 : 1.8} />
        <path d="M12 7v5l3 3" stroke="currentColor" strokeWidth={active ? 2.2 : 1.8} strokeLinecap="round" />
      </svg>
    ),
  },
  {
    to: '/matches',
    label: 'Matches',
    icon: (active) => (
      <svg width="24" height="24" viewBox="0 0 24 24" fill={active ? 'currentColor' : 'none'}>
        <path
          d="M12 21C12 21 3 15 3 8.5A5 5 0 0 1 12 6a5 5 0 0 1 9 2.5C21 15 12 21 12 21z"
          stroke="currentColor" strokeWidth={active ? 0 : 1.8} strokeLinecap="round" strokeLinejoin="round"
        />
      </svg>
    ),
  },
  {
    to: '/profile',
    label: 'Profile',
    icon: (active) => (
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none">
        <circle cx="12" cy="8" r="4" stroke="currentColor" strokeWidth={active ? 2.2 : 1.8} />
        <path d="M4 20c0-4 3.6-7 8-7s8 3 8 7" stroke="currentColor" strokeWidth={active ? 2.2 : 1.8} strokeLinecap="round" />
      </svg>
    ),
  },
]

const TABBED_PATHS = ['/discover', '/matches', '/profile']

export default function NavBar() {
  const { pathname } = useLocation()
  if (!TABBED_PATHS.includes(pathname)) return null

  return (
    <nav className="glass border-t border-apple-gray-2 safe-bottom shrink-0">
      <div className="flex">
        {TABS.map(({ to, label, icon }) => {
          const active = pathname === to
          return (
            <NavLink
              key={to}
              to={to}
              className="flex-1 flex flex-col items-center gap-0.5 py-2.5"
            >
              <motion.span
                animate={{ color: active ? '#007AFF' : '#6E6E73' }}
                transition={{ duration: 0.15 }}
              >
                {icon(active)}
              </motion.span>
              <motion.span
                animate={{ color: active ? '#007AFF' : '#6E6E73' }}
                className="text-[10px] font-medium"
                transition={{ duration: 0.15 }}
              >
                {label}
              </motion.span>
            </NavLink>
          )
        })}
      </div>
    </nav>
  )
}
