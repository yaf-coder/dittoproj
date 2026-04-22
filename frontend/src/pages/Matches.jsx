import { useEffect, useState } from 'react'
import { motion } from 'framer-motion'
import { api } from '../api/client'

function MatchCard({ match, index }) {
  const { user } = match
  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ delay: index * 0.06, duration: 0.4, ease: [0.25, 0.46, 0.45, 0.94] }}
      className="flex items-center gap-4 p-4 rounded-3xl bg-white shadow-apple-sm active:bg-apple-gray transition-colors"
    >
      {/* Avatar */}
      <div className="w-16 h-16 rounded-full overflow-hidden bg-apple-gray shrink-0">
        {user.picture ? (
          <img src={user.picture} alt={user.name} className="w-full h-full object-cover" />
        ) : (
          <div className="w-full h-full flex items-center justify-center">
            <span className="text-2xl font-bold text-apple-sub/40">{user.name?.[0]}</span>
          </div>
        )}
      </div>

      {/* Info */}
      <div className="flex-1 min-w-0">
        <p className="font-semibold text-[17px] text-apple-text truncate">{user.name}</p>
        {user.bio && (
          <p className="text-apple-sub text-[14px] truncate mt-0.5">{user.bio}</p>
        )}
      </div>

      {/* Chevron */}
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" className="text-apple-sub/40 shrink-0">
        <path d="M9 18l6-6-6-6" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"/>
      </svg>
    </motion.div>
  )
}

export default function Matches() {
  const [matches, setMatches] = useState([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    api.getMatches()
      .then(setMatches)
      .finally(() => setLoading(false))
  }, [])

  return (
    <div className="flex-1 flex flex-col overflow-hidden">
      <div className="px-6 pt-14 pb-4 safe-top">
        <h1 className="text-[28px] font-bold tracking-tight2 text-apple-text">Matches</h1>
      </div>

      <div className="flex-1 overflow-y-auto px-4 pb-4">
        {loading ? (
          <div className="flex justify-center pt-20">
            <div className="w-8 h-8 border-[2.5px] border-apple-blue border-t-transparent rounded-full animate-spin" />
          </div>
        ) : matches.length === 0 ? (
          <div className="flex flex-col items-center justify-center pt-20 gap-3">
            <div className="w-20 h-20 rounded-full bg-apple-gray flex items-center justify-center">
              <svg width="36" height="36" viewBox="0 0 24 24" fill="none">
                <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2" stroke="#6E6E73" strokeWidth="1.8" strokeLinecap="round"/>
                <circle cx="9" cy="7" r="4" stroke="#6E6E73" strokeWidth="1.8"/>
                <path d="M23 21v-2a4 4 0 0 0-3-3.87M16 3.13a4 4 0 0 1 0 7.75" stroke="#6E6E73" strokeWidth="1.8" strokeLinecap="round"/>
              </svg>
            </div>
            <p className="text-apple-sub font-medium text-[17px]">No matches yet</p>
            <p className="text-apple-sub/60 text-[14px] text-center px-8">
              Keep swiping — your matches will appear here.
            </p>
          </div>
        ) : (
          <div className="flex flex-col gap-2">
            {matches.map((m, i) => (
              <MatchCard key={m.match_id} match={m} index={i} />
            ))}
          </div>
        )}
      </div>
    </div>
  )
}
